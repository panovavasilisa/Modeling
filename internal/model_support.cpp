#include "model_support.h"
#include "back/data.h"
#include <algorithm>
#include <cmath>
#include <stdexcept>

namespace model_detail {

void DiscountPolicy::validate() const {
    for (double value : {card_percent, large_purchase_percent, regular_percent}) {
        if (!std::isfinite(value) || value < 0.0 || value > 100.0)
            throw std::invalid_argument("Discount percentages must be between 0 and 100");
    }
    if (!std::isfinite(maximum_percent) || maximum_percent < 0.0 || maximum_percent > 9.0)
        throw std::invalid_argument("The total discount limit must be between 0 and 9");
    if (!std::isfinite(large_purchase_threshold) || large_purchase_threshold < 0.0)
        throw std::invalid_argument("Invalid large purchase threshold");
}

double DiscountPolicy::percent_for(const Customer& customer, double subtotal) const {
    validate();
    double percent = customer.has_card ? card_percent
        : (subtotal > large_purchase_threshold ? large_purchase_percent : 0.0);
    if (customer.is_regular) percent += regular_percent;
    return std::min(percent, maximum_percent);
}

ModelContext& context() {
    static ModelContext instance;
    return instance;
}

void ModelContext::reset(std::uint32_t seed) {
    random.seed(seed);
    discounts = DiscountPolicy{};
    current_day = 0;
    delivery_lead_days = 0;
    initial_count = 80;
    warehouses.clear();
    last_fulfillment.clear();
}

void validate_medicine_id(int id) {
    if (id < 0 || static_cast<std::size_t>(id) >= mas_med.size())
        throw std::invalid_argument("Unknown medicine index");
}

void validate_batch(const StoreBatch& batch) {
    validate_medicine_id(batch.mas_med_id);
    if (batch.count < 0 || batch.expiration_day < 0 ||
        !std::isfinite(batch.batch_price) || batch.batch_price < 0.0 ||
        !std::isfinite(batch.count * batch.batch_price))
        throw std::invalid_argument("Invalid warehouse batch");
}

WarehouseAccounting& ModelContext::accounting(Warehouse& warehouse) {
    auto& result = warehouses[&warehouse];
    // Сопоставляем партии по значениям, а не по адресам элементов вектора.
    // Изменение количества через StoreBatch::take не меняет закупочную цену партии.
    std::vector<BatchAccounting> next;
    next.reserve(warehouse.inventory.size());
    std::vector<bool> used(result.batches.size(), false);
    for (const auto& batch : warehouse.inventory) {
        validate_batch(batch);
        auto match = result.batches.size();
        for (std::size_t i = 0; i < result.batches.size(); ++i) {
            const auto& old = result.batches[i];
            if (!used[i] && old.medicine_id == batch.mas_med_id &&
                old.expiration_day == batch.expiration_day &&
                old.selling_base == batch.batch_price) {
                match = i;
                break;
            }
        }
        if (match != result.batches.size()) {
            used[match] = true;
            next.push_back(result.batches[match]);
        } else {
            next.push_back({batch.mas_med_id, batch.expiration_day,
                batch.batch_price, batch.batch_price, false});
        }
    }
    result.batches = std::move(next);
    return result;
}

double ModelContext::inventory_cost(Warehouse& warehouse) {
    auto& ledger = accounting(warehouse);
    double result = 0.0;
    for (std::size_t i = 0; i < warehouse.inventory.size(); ++i)
        result += warehouse.inventory[i].count * ledger.batches[i].acquisition_price;
    return result;
}

double ModelContext::collect_write_off(Warehouse& warehouse) {
    auto& ledger = accounting(warehouse);
    const double result = ledger.pending_write_off;
    ledger.pending_write_off = 0.0;
    return result;
}

bool ModelContext::is_discounted(Warehouse& warehouse, int medicine_id) {
    const auto& ledger = accounting(warehouse);
    for (std::size_t i = 0; i < warehouse.inventory.size(); ++i)
        if (warehouse.inventory[i].mas_med_id == medicine_id &&
            warehouse.inventory[i].count > 0 &&
            warehouse.inventory[i].expiration_day >= current_day &&
            ledger.batches[i].discounted)
            return true;
    return false;
}

int random_int(int low, int high) {
    if (low > high) throw std::invalid_argument("Invalid random range");
    return std::uniform_int_distribution<int>(low, high)(context().random);
}

} // пространство имён model_detail
