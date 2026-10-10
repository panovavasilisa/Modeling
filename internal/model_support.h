#pragma once

#include <cstdint>
#include <random>
#include <unordered_map>
#include <utility>
#include <vector>
#include "back/customer.h"
#include "back/store.h"

// Внутреннее состояние реализации; объявления исходных классов сохраняются.
namespace model_detail {

struct DiscountPolicy {
    double card_percent = 5.0;
    double large_purchase_percent = 3.0;
    double regular_percent = 5.0;
    double maximum_percent = 9.0;
    double large_purchase_threshold = 1000.0;
    void validate() const;
    double percent_for(const Customer& customer, double subtotal) const;
};

struct BatchAccounting {
    int medicine_id;
    int expiration_day;
    double acquisition_price;
    double selling_base;
    bool discounted;
};

struct WarehouseAccounting {
    std::vector<BatchAccounting> batches;
    double pending_write_off = 0.0;
    double sold_cost = 0.0;
    double total_write_off = 0.0;
    std::int64_t sold_packages = 0;
    std::int64_t written_off_packages = 0;
};

class ModelContext {
public:
    DiscountPolicy discounts;
    std::mt19937 random{std::random_device{}()};
    int current_day = 0;
    int delivery_lead_days = 0;
    int initial_count = 80;
    std::unordered_map<const Warehouse*, WarehouseAccounting> warehouses;
    std::vector<std::pair<int, int>> last_fulfillment;

    void reset(std::uint32_t seed);
    WarehouseAccounting& accounting(Warehouse& warehouse);
    double inventory_cost(Warehouse& warehouse);
    double collect_write_off(Warehouse& warehouse);
    bool is_discounted(Warehouse& warehouse, int medicine_id);
};

ModelContext& context();
void validate_medicine_id(int id);
void validate_batch(const StoreBatch& batch);
int random_int(int low, int high);

} // пространство имён model_detail
