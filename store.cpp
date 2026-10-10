#include "back/store.h"
#include "back/data.h"
#include "internal/model_support.h"
#include <cmath>
#include <cstdint>
#include <stdexcept>

double StoreBatch::price() {
    if (count < 0 || !std::isfinite(batch_price) || batch_price < 0.0)
        throw std::invalid_argument("Invalid batch price or count");
    const double total = count * batch_price;
    if (!std::isfinite(total)) throw std::overflow_error("Batch price overflow");
    return total;
}

int StoreBatch::take(int cnt) {
    if (cnt < 0 || count < 0) throw std::invalid_argument("Negative package count");
    const int supplied = cnt < count ? cnt : count;
    count -= supplied;
    return supplied;
}

void Warehouse::add_batch(const StoreBatch& batch) {
    model_detail::validate_batch(batch);
    auto& ledger = model_detail::context().accounting(*this);
    inventory.push_back(batch);
    ledger.batches.push_back({batch.mas_med_id, batch.expiration_day,
        batch.batch_price, batch.batch_price, false});
}

void Warehouse::write_off(int current_day) {
    if (current_day < 0) throw std::invalid_argument("Negative simulation day");
    auto& ledger = model_detail::context().accounting(*this);
    std::vector<StoreBatch> kept;
    std::vector<model_detail::BatchAccounting> kept_costs;
    kept.reserve(inventory.size());
    kept_costs.reserve(inventory.size());
    for (std::size_t i = 0; i < inventory.size(); ++i) {
        auto batch = inventory[i];
        auto cost = ledger.batches[i];
        if (batch.expiration_day < current_day) {
            const double loss = batch.count * cost.acquisition_price;
            ledger.pending_write_off += loss;
            ledger.total_write_off += loss;
            ledger.written_off_packages += batch.count;
            continue;
        }
        if (batch.expiration_day - current_day <= 30 && !cost.discounted) {
            batch.batch_price *= 0.5;
            cost.selling_base = batch.batch_price;
            cost.discounted = true;
        }
        if (batch.count > 0) {
            kept.push_back(batch);
            kept_costs.push_back(cost);
        }
    }
    inventory = std::move(kept);
    ledger.batches = std::move(kept_costs);
}

bool Warehouse::need_restock() {
    std::vector<std::int64_t> counts(mas_med.size(), 0);
    for (const auto& batch : inventory) {
        model_detail::validate_batch(batch);
        if (batch.expiration_day >= model_detail::context().current_day)
            counts[batch.mas_med_id] += batch.count;
    }
    for (std::size_t i = 0; i < mas_med.size(); ++i) {
        if (mas_med[i].min_stock < 0) throw std::invalid_argument("Negative minimum stock");
        if (counts[i] < mas_med[i].min_stock) return true;
    }
    return false;
}
