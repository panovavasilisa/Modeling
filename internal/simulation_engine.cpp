#include "simulation_engine.h"
#include "model_support.h"
#include "back/data.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>

void SimulationParameters::validate() const {
    if (days < 10 || days > 25) throw std::invalid_argument("N must be between 10 and 25");
    if (couriers < 3 || couriers > 9) throw std::invalid_argument("M must be between 3 and 9");
    if (medicine_count < 15 || medicine_count > 35) throw std::invalid_argument("K must be between 15 and 35");
    if (!std::isfinite(markup_percent) || markup_percent < 0 || markup_percent > 10000)
        throw std::invalid_argument("Retail markup must be between 0 and 10000 percent");
    if (initial_count < 0 || initial_count > 1000000)
        throw std::invalid_argument("Initial packages per medicine must be between 0 and 1000000");
    if (regular_customers < 0 || regular_customers > 100000)
        throw std::invalid_argument("Regular customer count must be between 0 and 100000");
    model_detail::DiscountPolicy{card_discount, large_purchase_discount, regular_discount,
                                maximum_discount, 1000.0}.validate();
}

namespace model_detail {

SimulationEngine& engine() {
    static SimulationEngine instance;
    return instance;
}

void SimulationEngine::configure(const SimulationParameters& parameters) {
    parameters.validate();
    if (phase_ != Phase::uninitialized && phase_ != Phase::finished)
        throw std::logic_error("Finish the current day before reconfiguring");
    parameters_ = parameters;
    phase_ = Phase::uninitialized;
}

void SimulationEngine::load_initial_stock() {
    if (parameters_.initial_stock_file.empty()) {
        generator_.generate_warehouse(0);
        return;
    }
    std::ifstream input(parameters_.initial_stock_file);
    if (!input) throw std::runtime_error("Cannot open initial stock file");
    std::vector<StoreBatch> batches;
    std::string line;
    int line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        auto comment = line.find('#');
        if (comment != std::string::npos) line.erase(comment);
        std::replace(line.begin(), line.end(), ',', ' ');
        std::istringstream row(line);
        row >> std::ws;
        if (row.eof()) continue;
        if (line.find("medicine_id") != std::string::npos && batches.empty()) continue;
        StoreBatch batch{};
        if (!(row >> batch.mas_med_id >> batch.count >> batch.expiration_day))
            throw std::runtime_error("Invalid initial stock row " + std::to_string(line_number));
        validate_medicine_id(batch.mas_med_id);
        row >> std::ws;
        if (row.eof()) batch.batch_price = mas_med[batch.mas_med_id].wholesale_price;
        else if (!(row >> batch.batch_price))
            throw std::runtime_error("Invalid initial stock price on row " + std::to_string(line_number));
        row >> std::ws;
        if (!row.eof()) throw std::runtime_error("Extra initial stock columns on row " + std::to_string(line_number));
        validate_batch(batch);
        batches.push_back(batch);
    }
    warehouse.inventory.clear();
    context().warehouses.erase(&warehouse);
    for (const auto& batch : batches) warehouse.add_batch(batch);
}

void SimulationEngine::initialize() {
    parameters_.validate();
    context().reset(parameters_.seed);
    context().discounts = {parameters_.card_discount, parameters_.large_purchase_discount,
        parameters_.regular_discount, parameters_.maximum_discount, 1000.0};
    context().initial_count = parameters_.initial_count;
    context().delivery_lead_days = 1;
    N = parameters_.days;
    M = parameters_.couriers;
    K = parameters_.medicine_count;
    global_marcup = parameters_.markup_percent;
    warehouse.inventory.clear();
    customers.clear();
    result_ = SimulationResult{};
    stats_ = Statistics{};
    orders_.clear();
    tomorrow_.clear();
    day_ = 0;
    next_id_ = 1;
    delivered_cost_ = 0.0;
    generator_.generate_catalog(K);
    load_initial_stock();
    for (const auto& batch : warehouse.inventory) result_.initial_packages += batch.count;
    stats_.update(0, context().inventory_cost(warehouse), 0);
    // Уцениваем начальные партии перед формированием списков регулярных покупок.
    warehouse.write_off(0);
    generator_.generate_customers(parameters_.regular_customers);
    supplier_ = std::make_unique<SupplyManager>(warehouse, stats_, generator_);
    phase_ = Phase::finished;
    update_result();
}

void SimulationEngine::require_phase(Phase expected, int day) const {
    if (phase_ != expected || day != day_) throw std::logic_error("Invalid daily simulation sequence");
}

void SimulationEngine::begin_day(int day) {
    if (phase_ != Phase::finished || day != day_ + 1 || day > N)
        throw std::logic_error("Simulation days must advance once, in order, up to N");
    day_ = day;
    context().current_day = day;
    today_ = DailySimulationResult{};
    today_.day = day;
    today_.courier_load.assign(M, 0);
    warehouse.write_off(day);
    stats_.update(0, 0, context().collect_write_off(warehouse));
    today_.arrived_supplies = supplier_->advance_to(day);
    phase_ = Phase::begun;
}

void SimulationEngine::generate_events(int day) {
    require_phase(Phase::begun, day);
    for (std::size_t i = 0; i < customers.size(); ++i) {
        auto& customer = customers[i];
        if (!customer.is_regular) continue;
        if (customer.periodicity <= 0) throw std::invalid_argument("Invalid recurring purchase period");
        if (customer.next_purchase_day <= day) {
            orders_.push_back({next_id_++, day, Order{static_cast<int>(i), customer.regular_purchases, 0}});
            do { customer.next_purchase_day += customer.periodicity; } while (customer.next_purchase_day <= day);
            ++today_.planned_orders;
            ++today_.received_orders;
        }
    }
    // Вероятность поступления заказов не зависит от M: число курьеров определяет
    // вместимость службы доставки, а не создаёт дополнительный спрос.
    for (int trial = 0; trial < 180; ++trial) {
        if (!generator_.should_generate_order(global_marcup)) continue;
        generator_.generate_random_customers();
        orders_.push_back({next_id_++, day, Order{static_cast<int>(customers.size() - 1),
                                               generator_.generate_order_items(), 0}});
        ++today_.received_orders;
    }
    phase_ = Phase::generated;
}

void SimulationEngine::process_orders(int day) {
    require_phase(Phase::generated, day);
    const int capacity = 15 * M;
    if (tomorrow_.size() > static_cast<std::size_t>(capacity))
        throw std::logic_error("Prepared deliveries exceed courier capacity");
    // Доставляем покупки, выделенные вчера. Доход учитываем при доставке;
    // данные покупателя сохраняются после удаления разового клиента из списка.
    for (std::size_t i = 0; i < tomorrow_.size(); ++i) {
        auto purchase = std::move(tomorrow_[i]);
        const int courier = static_cast<int>((i + day - 1) % M);
        purchase.delivered_day = day;
        purchase.courier_index = courier;
        ++today_.courier_load[courier];
        ++today_.delivered_orders;
        today_.late_deliveries += day > purchase.ordered_day + 1;
        stats_.update(purchase.price, 0, 0);
        delivered_cost_ += purchase.acquisition_cost;
        result_.completed_orders.push_back(std::move(purchase));
    }
    tomorrow_.clear();
    today_.courier_limits_satisfied = true;
    // Проверяем заказы за день, назначенные каждому курьеру по его индексу.
    std::vector<int> actual_load(M, 0);
    for (const auto& purchase : result_.completed_orders)
        if (purchase.delivered_day == day) ++actual_load.at(purchase.courier_index);
    for (int courier = 0; courier < M; ++courier) {
        if (actual_load[courier] != today_.courier_load[courier] || actual_load[courier] > 15)
            throw std::logic_error("Invalid courier assignment");
        if (actual_load[courier] < 7) today_.courier_limits_satisfied = false;
    }
    while (!orders_.empty() && tomorrow_.size() < static_cast<std::size_t>(capacity)) {
        auto queued = std::move(orders_.front());
        orders_.pop_front();
        Statistics reserved_sale{};
        const double before = context().accounting(warehouse).sold_cost;
        if (!queued.order.process_order(warehouse, reserved_sale, global_marcup)) {
            ++today_.empty_orders;
            continue;
        }
        const double acquisition = context().accounting(warehouse).sold_cost - before;
        tomorrow_.push_back({queued.id, queued.ordered_day, day, 0, -1,
            customers.at(queued.order.customer_idx), queued.order.items,
            context().last_fulfillment, queued.order.final_price, acquisition});
        ++today_.prepared_orders;
    }
    today_.created_supplies = supplier_->create_requests(day);
    phase_ = Phase::processed;
}

void SimulationEngine::finish_day() {
    require_phase(Phase::processed, day_);
    // Сохраняем клиентов из очереди и пересчитываем индексы после удаления остальных.
    // Подготовленные и выполненные покупки уже содержат копии данных покупателей.
    std::vector<bool> retain(customers.size(), false);
    for (const auto& queued : orders_) retain.at(queued.order.customer_idx) = true;
    std::vector<int> mapping(customers.size(), -1);
    std::vector<Customer> remaining;
    for (std::size_t i = 0; i < customers.size(); ++i) {
        if (!customers[i].is_regular && !retain[i]) continue;
        mapping[i] = static_cast<int>(remaining.size());
        remaining.push_back(std::move(customers[i]));
    }
    for (auto& queued : orders_) queued.order.customer_idx = mapping.at(queued.order.customer_idx);
    customers = std::move(remaining);
    today_.pending_orders = static_cast<int>(orders_.size());
    today_.available_packages.assign(mas_med.size(), 0);
    for (const auto& batch : warehouse.inventory) today_.available_packages[batch.mas_med_id] += batch.count;
    today_.totals = stats_;
    result_.days.push_back(today_);
    phase_ = Phase::finished;
    update_result();
}

void SimulationEngine::update_result() {
    result_.totals = stats_;
    result_.available_stock_cost = context().inventory_cost(warehouse);
    result_.reserved_stock_cost = 0.0;
    for (const auto& purchase : tomorrow_) result_.reserved_stock_cost += purchase.acquisition_cost;
    result_.delivered_stock_cost = delivered_cost_;
    result_.profit = stats_.income - stats_.purchase_expenses + result_.available_stock_cost + result_.reserved_stock_cost;
    result_.written_off_packages = context().accounting(warehouse).written_off_packages;
    result_.supplied_packages = 0;
    result_.pending_supplies.clear();
    if (supplier_) {
        for (const auto& receipt : supplier_->receipts()) result_.supplied_packages += receipt.count;
        for (const auto& supply : supplier_->pending()) result_.pending_supplies.push_back(supply.request);
    }
    result_.pending_deliveries = tomorrow_.size();
    result_.pending_orders = orders_.size();
    const double other_profit = stats_.income - delivered_cost_ - stats_.write_off_losses;
    if (std::abs(result_.profit - other_profit) > 1e-7 * std::max(1.0, stats_.purchase_expenses))
        throw std::logic_error("Financial balance failed");
}

void SimulationEngine::write_summary(std::ostream& output) const {
    output << std::fixed << std::setprecision(2)
           << "\nИтоги за " << result_.days.size() << " дней (seed=" << parameters_.seed << ")\n"
           << "Доход доставленных заказов: " << result_.totals.income << " руб.\n"
           << "Расходы на закупку (включая начальный запас): " << result_.totals.purchase_expenses << " руб.\n"
           << "Потери от списания: " << result_.totals.write_off_losses << " руб.\n"
           << "Прибыль: " << result_.profit << " руб.\n"
           << "Стоимость оставшегося запаса: " << result_.available_stock_cost << " руб.\n"
           << "Стоимость выделенного товара до доставки: " << result_.reserved_stock_cost << " руб.\n"
           << "Доставлено заказов: " << result_.completed_orders.size() << '\n'
           << "Ожидают следующего дня: " << result_.pending_deliveries << '\n'
           << "Очередь из-за ограничения вместимости: " << result_.pending_orders << '\n'
           << "Ожидают поставки: " << result_.pending_supplies.size() << '\n';
    int feasible_days = 0;
    for (const auto& day : result_.days) feasible_days += day.courier_limits_satisfied;
    output << "Дней с нагрузкой 7–15 у всех курьеров: " << feasible_days << '/' << result_.days.size() << '\n';
    for (int courier = 0; courier < parameters_.couriers; ++courier) {
        int total = 0;
        output << "Курьер " << courier + 1 << ": ";
        for (const auto& day : result_.days) {
            const int load = day.courier_load[courier];
            output << load << ' ';
            total += load;
        }
        output << "(всего " << total << ")\n";
    }
}

} // пространство имён model_detail

void configure_simulation(const SimulationParameters& parameters) { model_detail::engine().configure(parameters); }
const SimulationParameters& simulation_parameters() { return model_detail::engine().parameters(); }
const SimulationResult& simulation_result() { return model_detail::engine().result(); }
void init_simulation() { model_detail::engine().initialize(); }
void generate_daily_events(int current_day) { model_detail::engine().generate_events(current_day); }
void process_daily_order(int current_day) { model_detail::engine().process_orders(current_day); }
void manage_warehouse(int current_day) { model_detail::engine().begin_day(current_day); }
void cleanup_random_customers() { model_detail::engine().finish_day(); }
void print_final_statistics() { model_detail::engine().write_summary(std::cout); }
