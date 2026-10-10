#include "back/data.h"
#include "back/data_generator.h"
#include "back/order.h"
#include "internal/model_support.h"
#include "internal/supply_manager.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {
int checks = 0;
void check(bool condition, const char* message) {
    ++checks;
    if (!condition) throw std::runtime_error(message);
}
void close(double actual, double expected, const char* message) {
    check(std::abs(actual - expected) < 1e-7 * std::max(1.0, std::abs(expected)), message);
}
template<class F> void rejects(F operation, const char* message) {
    bool threw = false;
    try { operation(); } catch (const std::exception&) { threw = true; }
    check(threw, message);
}
void reset() {
    model_detail::context().reset(42);
    warehouse.inventory.clear();
    mas_med = {Medicine{100, 0, 0, 100.0, 90, 5}, Medicine{200, 1, 0, 200.0, 90, 3}};
    customers = {Customer{}};
}
void batches_and_orders() {
    reset();
    StoreBatch batch{0, 8, 90, 100.0};
    close(batch.price(), 800, "batch total");
    check(batch.take(3) == 3 && batch.count == 5, "full take");
    check(batch.take(20) == 5 && batch.count == 0, "partial take");
    check(batch.take(1) == 0, "empty take");
    rejects([&] { batch.take(-1); }, "reject negative take");

    warehouse.add_batch({0, 3, 70, 100.0});
    warehouse.add_batch({0, 4, 40, 80.0});
    Statistics stats{};
    Order order{0, {{0, 6}}, 0};
    check(order.process_order(warehouse, stats, 25), "fulfilled order");
    check(warehouse.inventory[0].count == 1 && warehouse.inventory[1].count == 0, "FEFO across batches");
    close(order.final_price, (4 * 80 + 2 * 100) * 1.25, "actual price and markup");
    close(stats.income, order.final_price, "order income");
    close(model_detail::context().accounting(warehouse).sold_cost, 520, "sold acquisition cost");
    Order partial{0, {{0, 100}, {1, 3}}, 0};
    check(partial.process_order(warehouse, stats, 0), "partial order returns supplied");
    close(partial.final_price, 100, "partial price");
    check(model_detail::context().last_fulfillment == std::vector<std::pair<int,int>>{{0,1}}, "actual fulfillment");
    Order empty{0, {{1, 10}}, 17};
    check(!empty.process_order(warehouse, stats, 0), "absent order");
    close(empty.final_price, 0, "absent order costs zero");

    reset();
    warehouse.add_batch({0, 4, 50, 100});
    Order duplicate{0, {{0,3},{0,3}}, 0};
    Statistics duplicate_stats{};
    duplicate.process_order(warehouse, duplicate_stats, 0);
    close(duplicate.final_price, 400, "duplicate order positions cannot oversell");
    check(warehouse.inventory[0].count == 0, "duplicate stock conservation");
    Order invalid{0, {{0,1},{100,1}}, 0};
    rejects([&] { invalid.process_order(warehouse, duplicate_stats, 0); }, "invalid medicine");
    close(duplicate_stats.income, 400, "failed order cannot change income");
    rejects([&] { warehouse.add_batch({0,-1,50,100}); }, "invalid batch");
}
void discounts() {
    struct Scenario { bool card; bool regular; int count; double markup; double expected; };
    for (auto scenario : {Scenario{false,false,10,0,1000},
                          Scenario{false,false,11,0,1067},
                          Scenario{true,false,12,0,1140},
                          Scenario{false,true,12,0,1104},
                          Scenario{true,true,12,0,1092},
                          Scenario{false,false,8,25,1000},
                          Scenario{true,true,4,25,455}}) {
        reset();
        customers[0].has_card = scenario.card;
        customers[0].is_regular = scenario.regular;
        warehouse.add_batch({0, 20, 90, 100});
        Statistics stats{};
        Order order{0, {{0, scenario.count}}, 0};
        order.process_order(warehouse, stats, scenario.markup);
        close(order.final_price, scenario.expected, "discount rule");
    }
    reset();
    customers[0].has_card = customers[0].is_regular = true;
    model_detail::context().discounts.card_percent = 2;
    model_detail::context().discounts.regular_percent = 1;
    warehouse.add_batch({0, 5, 90, 100});
    Statistics stats{};
    Order custom{0, {{0,5}}, 0};
    custom.process_order(warehouse, stats, 20);
    close(custom.final_price, 582, "configurable discounts");
    model_detail::context().discounts.maximum_percent = 10;
    rejects([&] { custom.process_order(warehouse, stats, 20); }, "maximum 9 percent");
}
void expiration_and_restock() {
    reset();
    // A custom batch price must also be discounted exactly once.
    warehouse.add_batch({0, 4, 40, 250});
    warehouse.write_off(9);
    close(warehouse.inventory[0].batch_price, 250, "no early markdown");
    warehouse.write_off(10);
    close(warehouse.inventory[0].batch_price, 125, "30 day markdown");
    warehouse.write_off(11);
    warehouse.write_off(11);
    close(warehouse.inventory[0].batch_price, 125, "no repeated markdown");
    // Reallocation and equal medication indices do not lose markdown state.
    warehouse.add_batch({0, 3, 60, 100});
    warehouse.write_off(40);
    check(warehouse.inventory.size() == 2, "valid through expiration day");
    close(warehouse.inventory[0].batch_price, 125, "markdown survives addition");
    warehouse.write_off(41);
    check(warehouse.inventory.size() == 1, "expired batch removed");
    close(model_detail::context().collect_write_off(warehouse), 1000, "loss at acquisition cost");
    close(model_detail::context().collect_write_off(warehouse), 0, "loss collected once");
    check(warehouse.need_restock(), "missing medicine triggers restock");
    warehouse.add_batch({0, 2, 90, 100});
    warehouse.add_batch({1, 3, 90, 200});
    check(!warehouse.need_restock(), "sum across batches at threshold");
    warehouse.inventory[0].take(1);
    check(warehouse.need_restock(), "below threshold");

    reset();
    warehouse.add_batch({0, 3, 1, 100});
    model_detail::context().current_day = 1;
    model_detail::context().delivery_lead_days = 1;
    Statistics stats{};
    Order order{0, {{0,3}}, 0};
    check(!order.process_order(warehouse, stats, 0), "cannot ship expiring stock for tomorrow");
    check(warehouse.inventory[0].count == 3, "unsuitable stock remains until writeoff");
}
void generation() {
    reset();
    DataGenerator generator;
    bool timers[4]{};
    for (int i = 0; i < 1000; ++i) {
        int timer = generator.generate_delivery_timer();
        check(timer >= 1 && timer <= 3, "supplier range");
        timers[timer] = true;
    }
    check(timers[1] && timers[2] && timers[3], "all supplier delays occur");
    for (int count : {15,35}) {
        generator.generate_catalog(count);
        check(mas_med.size() == static_cast<std::size_t>(count) && K == count, "catalog boundaries");
        generator.generate_warehouse(0);
        check(warehouse.inventory.size() == static_cast<std::size_t>(2*count), "initial batches");
        generator.generate_customers(10);
        for (const auto& customer : customers)
            check(customer.is_regular && customer.periodicity > 0 &&
                  customer.next_purchase_day > 0 && !customer.regular_purchases.empty(), "regular customer schedule");
        generator.generate_random_customers();
        check(!customers.back().is_regular && customers.back().regular_purchases.empty(), "ordinary customer");
        for (int trial = 0; trial < 100; ++trial) {
            auto items = generator.generate_order_items();
            check(!items.empty() && items.size() <= 4, "random positions");
            for (const auto& item : items)
                check(item.first >= 0 && item.first < count && item.second >= 1 && item.second <= 4, "random item bounds");
        }
    }
    rejects([&] { generator.generate_catalog(14); }, "invalid K lower");
    rejects([&] { generator.generate_catalog(36); }, "invalid K upper");

    int low = 0, high = 0;
    model_detail::context().reset(123);
    for (int i = 0; i < 10000; ++i) low += generator.should_generate_order(10);
    model_detail::context().reset(123);
    for (int i = 0; i < 10000; ++i) high += generator.should_generate_order(90);
    check(low > 2 * high, "markup reduces arrival intensity");

    reset();
    generator.generate_catalog(15);
    warehouse.add_batch({0, 100, 10, mas_med[0].wholesale_price});
    warehouse.write_off(1);
    int discounted = 0, ordinary = 0;
    for (int i = 0; i < 10000; ++i)
        for (auto item : generator.generate_order_items()) {
            discounted += item.first == 0;
            ordinary += item.first == 1;
        }
    check(discounted > 2 * ordinary, "discounted medicine higher order probability");
}
void financial_balance() {
    reset();
    warehouse.add_batch({0,10,30,100});
    Statistics stats{};
    stats.update(0,1000,0);
    warehouse.write_off(1);
    Order order{0,{{0,4}},0};
    order.process_order(warehouse,stats,25);
    warehouse.write_off(31);
    stats.update(0,0,model_detail::context().collect_write_off(warehouse));
    close(stats.income,250,"discounted income");
    close(stats.purchase_expenses,1000,"procurement expenses");
    close(stats.write_off_losses,600,"writeoff losses");
    const double profit=stats.income-stats.purchase_expenses+model_detail::context().inventory_cost(warehouse);
    close(profit,-750,"cash plus stock profit");
    close(profit,stats.income-model_detail::context().accounting(warehouse).sold_cost-stats.write_off_losses,
          "losses not deducted twice");
    rejects([&]{stats.update(-1,0,0);},"negative statistics");
    close(stats.income,250,"failed update preserves stats");
}

void supplier_queue() {
    reset();
    Statistics stats{};
    DataGenerator generator;
    model_detail::SupplyManager supplier(warehouse, stats, generator);
    check(supplier.create_requests(1) == 2, "request each deficient medicine");
    const auto original = supplier.pending();
    check(supplier.create_requests(1) == 0, "no duplicate same-day requests");
    supplier.advance_to(1);
    check(warehouse.inventory.empty(), "no immediate supplier delivery");
    close(stats.purchase_expenses, 0, "only arrivals charged");
    for (int day = 2; day <= 4; ++day) {
        supplier.advance_to(day);
        const auto receipts = supplier.receipts().size();
        supplier.advance_to(day);
        check(supplier.receipts().size() == receipts, "idempotent supplier processing");
        check(supplier.create_requests(day) == 0, "no duplicate while waiting or sufficiently stocked");
        for (const auto& supply : original) {
            int available = 0;
            for (const auto& batch : warehouse.inventory)
                if (batch.mas_med_id == supply.request.mas_med_id) available += batch.count;
            check(available == (day >= supply.expected_day ? supply.request.count : 0), "exact supplier due day");
        }
    }
    check(supplier.pending().empty() && supplier.receipts().size() == 2, "all supplier requests fulfilled");
    close(stats.purchase_expenses, 15*100+9*200, "real supplier purchase expenses");
    for (const auto& receipt : supplier.receipts())
        check(receipt.arrived_day - receipt.ordered_day >= 1 && receipt.arrived_day - receipt.ordered_day <= 3,
              "receipt within one to three days");
    check(!warehouse.need_restock(), "supplied medicine participates in stock");
    rejects([&] { supplier.advance_to(3); }, "supplier time cannot reverse");
}
}

int main() {
    try {
        batches_and_orders(); discounts(); expiration_and_restock(); generation(); financial_balance(); supplier_queue();
        std::cout << "Core checks passed: " << checks << '\n';
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "Core test failed after " << checks << " checks: " << error.what() << '\n';
        return 1;
    }
}
