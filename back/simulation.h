#pragma once

#include "customer.h"
#include "request.h"
#include "statistics.h"
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

// Объявления перенесены из main.cpp без изменения сигнатур.
void init_simulation();
void generate_daily_events(int current_day);
void process_daily_order(int current_day);
void manage_warehouse(int current_day);
void cleanup_random_customers();
void print_final_statistics();

struct SimulationParameters {
    int days = 15;
    int couriers = 5;
    int medicine_count = 25;
    double markup_percent = 25.0;
    double card_discount = 5.0;
    double large_purchase_discount = 3.0;
    double regular_discount = 5.0;
    double maximum_discount = 9.0;
    int initial_count = 80;
    int regular_customers = 10;
    std::uint32_t seed = 5489;
    std::string initial_stock_file;
    void validate() const;
};

struct CompletedPurchase {
    std::uint64_t order_id;
    int ordered_day;
    int prepared_day;
    int delivered_day;
    int courier_index;
    Customer customer;
    std::vector<std::pair<int, int>> requested;
    std::vector<std::pair<int, int>> supplied;
    double price;
    double acquisition_cost;
};

struct DailySimulationResult {
    int day = 0;
    int received_orders = 0;
    int planned_orders = 0;
    int empty_orders = 0;
    int prepared_orders = 0;
    int delivered_orders = 0;
    int late_deliveries = 0;
    int arrived_supplies = 0;
    int created_supplies = 0;
    int pending_orders = 0;
    bool courier_limits_satisfied = false;
    std::vector<int> courier_load;
    std::vector<std::int64_t> available_packages;
    Statistics totals{};
};

struct SimulationResult {
    Statistics totals{};
    double profit = 0.0;
    double available_stock_cost = 0.0;
    double reserved_stock_cost = 0.0;
    double delivered_stock_cost = 0.0;
    std::int64_t initial_packages = 0;
    std::int64_t supplied_packages = 0;
    std::int64_t written_off_packages = 0;
    std::size_t pending_deliveries = 0;
    std::size_t pending_orders = 0;
    std::vector<Request> pending_supplies;
    std::vector<DailySimulationResult> days;
    std::vector<CompletedPurchase> completed_orders;
};

// Графический интерфейс может настроить модель и выполнять её по дням,
// получая результаты без запуска консольного приложения.
void configure_simulation(const SimulationParameters& parameters);
const SimulationParameters& simulation_parameters();
const SimulationResult& simulation_result();
