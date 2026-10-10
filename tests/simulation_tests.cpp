#include "back/data.h"
#include "back/simulation.h"
#include "internal/model_support.h"
#include "internal/simulation_cli.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <stdexcept>

namespace {
int checks = 0;
void check(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
void close(double actual, double expected, const char* message) {
    check(std::abs(actual - expected) <= 1e-7 * std::max(1.0, std::abs(expected)), message);
}
template<class F> void rejects(F operation, const char* message) {
    bool threw = false;
    try { operation(); } catch (const std::exception&) { threw = true; }
    check(threw, message);
}
void day(int value) {
    manage_warehouse(value);
    generate_daily_events(value);
    process_daily_order(value);
    cleanup_random_customers();
}
SimulationResult experiment(const SimulationParameters& parameters) {
    configure_simulation(parameters);
    init_simulation();
    for (int value = 1; value <= parameters.days; ++value) day(value);
    return simulation_result();
}
void verify(const SimulationParameters& parameters, const SimulationResult& result) {
    check(result.days.size() == static_cast<std::size_t>(parameters.days), "exactly N steps");
    check(N == parameters.days && M == parameters.couriers && K == parameters.medicine_count, "original parameters used");
    double income = 0, delivered_cost = 0;
    std::vector<std::vector<int>> assignment(parameters.days, std::vector<int>(parameters.couriers, 0));
    std::int64_t delivered_packages = 0;
    for (const auto& purchase : result.completed_orders) {
        check(purchase.delivered_day == purchase.prepared_day + 1, "delivery next day after allocation");
        check(purchase.prepared_day >= purchase.ordered_day, "no delivery before order");
        check(purchase.delivered_day >= 2 && purchase.delivered_day <= parameters.days, "delivery within experiment");
        check(purchase.courier_index >= 0 && purchase.courier_index < parameters.couriers, "courier index in M");
        check(!purchase.customer.fio.empty() && !purchase.customer.phone.empty() && !purchase.customer.address.empty(), "customer snapshot after cleanup");
        check(!purchase.supplied.empty(), "no empty deliveries");
        for (const auto& item : purchase.supplied) {
            int requested = 0;
            for (const auto& request : purchase.requested) if (request.first == item.first) requested += request.second;
            check(item.second > 0 && item.second <= requested, "actual quantity never exceeds requested");
            delivered_packages += item.second;
        }
        ++assignment[purchase.delivered_day - 1][purchase.courier_index];
        income += purchase.price;
        delivered_cost += purchase.acquisition_cost;
    }
    int received = 0, prepared = 0, empty = 0;
    for (std::size_t i = 0; i < result.days.size(); ++i) {
        const auto& current = result.days[i];
        check(current.day == static_cast<int>(i + 1), "consecutive day indices");
        check(current.courier_load == assignment[i], "load derived from actual delivery list");
        check(current.courier_load.size() == static_cast<std::size_t>(parameters.couriers), "exactly M couriers");
        check(current.available_packages.size() == static_cast<std::size_t>(parameters.medicine_count), "exactly K stock totals");
        const int count = std::accumulate(current.courier_load.begin(), current.courier_load.end(), 0);
        check(count == current.delivered_orders, "load equals completed purchases");
        check(*std::max_element(current.courier_load.begin(), current.courier_load.end()) <= 15, "maximum fifteen per courier");
        if (count >= 7 * parameters.couriers) {
            check(*std::min_element(current.courier_load.begin(), current.courier_load.end()) >= 7, "minimum seven when feasible");
            check(current.courier_limits_satisfied, "feasible load reported");
        } else check(!current.courier_limits_satisfied, "underload reported honestly");
        for (auto amount : current.available_packages) check(amount >= 0, "nonnegative stock");
        if (i) {
            check(result.days[i].delivered_orders == result.days[i-1].prepared_orders, "only preceding-day purchases delivered");
            check(current.totals.income >= result.days[i-1].totals.income, "income monotonic");
        }
        received += current.received_orders;
        prepared += current.prepared_orders;
        empty += current.empty_orders;
    }
    check(result.days.front().delivered_orders == 0, "no invented day-zero orders");
    check(result.pending_deliveries == static_cast<std::size_t>(result.days.back().prepared_orders), "last-day deliveries remain pending");
    check(received == prepared + empty + static_cast<int>(result.pending_orders), "all received orders accounted for");
    close(income, result.totals.income, "income from delivered purchases");
    close(delivered_cost, result.delivered_stock_cost, "delivered acquisition cost");
    close(result.profit, income - delivered_cost - result.totals.write_off_losses, "profit losses once");
    close(result.totals.purchase_expenses, delivered_cost + result.available_stock_cost + result.reserved_stock_cost + result.totals.write_off_losses,
          "monetary stock conservation");
    const auto& ledger = model_detail::context().accounting(warehouse);
    const auto& last = result.days.back();
    const auto available = std::accumulate(last.available_packages.begin(), last.available_packages.end(), std::int64_t{0});
    check(result.initial_packages + result.supplied_packages == available + ledger.sold_packages + result.written_off_packages,
          "physical package conservation");
    check(delivered_packages <= ledger.sold_packages, "only allocated packages delivered");
    std::vector<bool> waiting(parameters.medicine_count, false);
    for (const auto& request : result.pending_supplies) {
        check(!waiting.at(request.mas_med_id), "no duplicate pending supplier orders");
        waiting[request.mas_med_id] = true;
        check(request.delivery_timer >= 1 && request.delivery_timer <= 3, "remaining supplier timer");
    }
}
void boundaries_and_replay() {
    for (int days : {10,25}) for (int couriers : {3,9}) for (int medicines : {15,35}) {
        SimulationParameters parameters;
        parameters.days = days;
        parameters.couriers = couriers;
        parameters.medicine_count = medicines;
        parameters.seed = 71;
        auto result = experiment(parameters);
        verify(parameters, result);
    }
    SimulationParameters parameters;
    parameters.days = 10;
    const auto first = experiment(parameters);
    const auto second = experiment(parameters);
    check(first.totals.income == second.totals.income && first.profit == second.profit, "repeat experiment same seed");
    check(first.completed_orders.size() == second.completed_orders.size(), "replay order count");
    for (std::size_t i = 0; i < first.completed_orders.size(); ++i)
        check(first.completed_orders[i].supplied == second.completed_orders[i].supplied &&
              first.completed_orders[i].customer.phone == second.completed_orders[i].customer.phone,
              "replay actual orders and customer data");
}
void recurring_and_overflow() {
    SimulationParameters parameters;
    parameters.days = 10;
    parameters.regular_customers = 0;
    parameters.markup_percent = 10000;
    configure_simulation(parameters);
    init_simulation();
    Customer customer{};
    customer.fio = "Покупатель \"плановый\"\nстрока";
    customer.phone = "+79000000000";
    customer.address = "Дом 1";
    customer.has_card = customer.is_regular = true;
    customer.card_number = 17;
    customer.periodicity = 3;
    customer.next_purchase_day = 2;
    customer.regular_purchases = {{0,2}};
    customers.push_back(customer);
    for (int value = 1; value <= parameters.days; ++value) day(value);
    const auto recurring = simulation_result();
    check(recurring.completed_orders.size() == 3, "periodic purchases generated");
    check(recurring.completed_orders[0].ordered_day == 2 && recurring.completed_orders[0].delivered_day == 3,
          "regular order and next-day delivery");
    check(recurring.completed_orders[1].ordered_day == 5 && recurring.completed_orders[2].ordered_day == 8,
          "regular periodicity preserved");
    check(customers.size() == 1 && customers[0].next_purchase_day == 11, "regular customer retained");

    parameters.markup_percent = 0;
    parameters.couriers = 3;
    parameters.initial_count = 100000;
    parameters.regular_customers = 100;
    const auto busy = experiment(parameters);
    verify(parameters, busy);
    check(busy.pending_orders > 0, "overload queued without overselling");
    bool late = false;
    for (const auto& current : busy.days) late |= current.late_deliveries > 0;
    check(late, "overload delay exposed");
    for (const auto& current : busy.days) if (current.day > 1)
        check(current.delivered_orders == 45 && current.courier_limits_satisfied, "busy three-courier day 15 each");
}
void empty_and_input(const std::string& temp_directory) {
    SimulationParameters parameters;
    parameters.days = 10;
    parameters.initial_count = 0;
    parameters.regular_customers = 0;
    parameters.markup_percent = 10000;
    auto result = experiment(parameters);
    verify(parameters, result);
    check(result.initial_packages == 0 && result.supplied_packages > 0, "empty warehouse restocked");
    check(result.completed_orders.empty(), "no invented sales under zero demand");
    close(result.profit, 0, "unsold purchases remain assets");

    const std::string path = temp_directory + "/initial-stock.csv";
    { std::ofstream stock(path); stock << "medicine_id,count,expiration_day,unit_price\n0,7,0,123\n1,4,60,200\n"; }
    parameters.initial_stock_file = path;
    configure_simulation(parameters);
    init_simulation();
    check(warehouse.inventory.size() == 2, "custom initial batch set");
    close(simulation_result().totals.purchase_expenses, 7*123+4*200, "custom initial stock cost");
    for (int value = 1; value <= parameters.days; ++value) day(value);
    result = simulation_result();
    verify(parameters, result);
    close(result.totals.write_off_losses, 7*123, "custom expiring batch loss");
    model_detail::write_json_report(temp_directory + "/report.json");
    { std::ofstream stock(path); stock << "0,7,60,not-a-price\n"; }
    configure_simulation(parameters);
    rejects([] { init_simulation(); }, "reject invalid stock file");
}
void invalid_parameters_and_sequence() {
    SimulationParameters parameters;
    for (int days : {9,26}) { parameters.days = days; rejects([&]{ configure_simulation(parameters); }, "invalid N"); }
    parameters.days = 10;
    for (int couriers : {2,10}) { parameters.couriers = couriers; rejects([&]{ configure_simulation(parameters); }, "invalid M"); }
    parameters.couriers = 3;
    for (int medicines : {14,36}) { parameters.medicine_count = medicines; rejects([&]{ configure_simulation(parameters); }, "invalid K"); }
    parameters.medicine_count = 15;
    parameters.markup_percent = std::numeric_limits<double>::quiet_NaN();
    rejects([&]{configure_simulation(parameters);}, "reject NaN");
    parameters.markup_percent = 25;
    parameters.maximum_discount = 10;
    rejects([&]{configure_simulation(parameters);}, "reject discount cap above nine");
    parameters.maximum_discount = 9;
    configure_simulation(parameters);
    init_simulation();
    rejects([] { manage_warehouse(2); }, "cannot skip first day");
    rejects([] { process_daily_order(1); }, "cannot process before generating");
    manage_warehouse(1);
    rejects([] { manage_warehouse(1); }, "cannot repeat beginning of day");
    rejects([&]{ configure_simulation(parameters); }, "cannot reconfigure mid-day");
    generate_daily_events(1); process_daily_order(1); cleanup_random_customers();
    rejects([] { cleanup_random_customers(); }, "cannot repeat end of day");
}
}
int main(int argc, char* argv[]) {
    try {
        check(argc == 2, "temporary directory supplied");
        invalid_parameters_and_sequence(); boundaries_and_replay(); recurring_and_overflow(); empty_and_input(argv[1]);
        std::cout << "Simulation checks passed: " << checks << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Simulation test failed after " << checks << " checks: " << error.what() << '\n';
        return 1;
    }
}
