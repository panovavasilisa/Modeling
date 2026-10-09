#include "supply_manager.h"
#include "model_support.h"
#include "back/data.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>

namespace model_detail {

SupplyManager::SupplyManager(Warehouse& warehouse, Statistics& stats, DataGenerator& generator)
    : warehouse_(warehouse), stats_(stats), generator_(generator) {}

int SupplyManager::create_requests(int current_day) {
    if (current_day < last_processed_day_) throw std::invalid_argument("Supply time cannot run backwards");
    context().current_day = current_day;
    if (!warehouse_.need_restock()) return 0;
    std::vector<std::int64_t> counts(mas_med.size(), 0);
    std::vector<bool> waiting(mas_med.size(), false);
    for (const auto& batch : warehouse_.inventory)
        if (batch.expiration_day >= current_day) counts[batch.mas_med_id] += batch.count;
    for (const auto& supply : pending_) waiting[supply.request.mas_med_id] = true;
    int created = 0;
    for (std::size_t i = 0; i < mas_med.size(); ++i) {
        if (counts[i] >= mas_med[i].min_stock || waiting[i]) continue;
        const std::int64_t amount = 3LL * mas_med[i].min_stock - counts[i];
        if (amount > std::numeric_limits<int>::max()) throw std::overflow_error("Restock quantity overflow");
        const int delay = generator_.generate_delivery_timer();
        pending_.push_back({Request{static_cast<int>(i), static_cast<int>(amount), delay},
                            current_day, current_day + delay});
        ++created;
    }
    return created;
}

int SupplyManager::advance_to(int current_day) {
    if (current_day < last_processed_day_) throw std::invalid_argument("Supply time cannot run backwards");
    int arrived = 0;
    std::vector<PendingSupply> waiting;
    for (auto supply : pending_) {
        const int elapsed = std::max(0, current_day - std::max(last_processed_day_, supply.ordered_day));
        supply.request.delivery_timer = std::max(0, supply.request.delivery_timer - elapsed);
        if (supply.request.delivery_timer > 0) {
            waiting.push_back(supply);
            continue;
        }
        const auto& medicine = mas_med.at(supply.request.mas_med_id);
        const double cost = supply.request.count * medicine.wholesale_price;
        if (medicine.shelf_life <= 0 || !std::isfinite(cost) || cost < 0.0)
            throw std::invalid_argument("Invalid supplier medicine");
        const StoreBatch batch{supply.request.mas_med_id, supply.request.count,
            current_day + medicine.shelf_life, medicine.wholesale_price};
        validate_batch(batch);
        stats_.update(0.0, cost, 0.0);
        warehouse_.add_batch(batch);
        receipts_.push_back({supply.request.mas_med_id, supply.request.count,
                            supply.ordered_day, current_day, cost});
        ++arrived;
    }
    pending_ = std::move(waiting);
    last_processed_day_ = current_day;
    return arrived;
}

} // namespace model_detail
