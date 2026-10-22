#include "back/order.h"
#include "back/data.h"
#include "internal/model_support.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <limits>
#include <numeric>
#include <stdexcept>

bool Order::process_order(Warehouse& warehouse, Statistics& stats, double markup_percent) {
    auto& state = model_detail::context();
    state.last_fulfillment.clear();
    if (customer_idx < 0 || static_cast<std::size_t>(customer_idx) >= customers.size())
        throw std::invalid_argument("Unknown customer index");
    if (!std::isfinite(markup_percent) || markup_percent < 0.0)
        throw std::invalid_argument("Invalid retail markup");
    state.discounts.validate();
    for (const auto& item : items) {
        model_detail::validate_medicine_id(item.first);
        if (item.second < 0) throw std::invalid_argument("Negative order quantity");
    }
    auto& ledger = state.accounting(warehouse);
    std::vector<std::size_t> indices(warehouse.inventory.size());
    std::iota(indices.begin(), indices.end(), 0);
    std::stable_sort(indices.begin(), indices.end(), [&](std::size_t a, std::size_t b) {
        return warehouse.inventory[a].expiration_day < warehouse.inventory[b].expiration_day;
    });
    std::vector<int> reserved(warehouse.inventory.size(), 0);
    std::vector<std::int64_t> supplied(mas_med.size(), 0);
    double subtotal = 0.0;
    double acquisition_cost = 0.0;
    std::int64_t total_packages = 0;
    for (const auto& item : items) {
        int remaining = item.second;
        for (auto index : indices) {
            const auto& batch = warehouse.inventory[index];
            if (batch.mas_med_id != item.first ||
                batch.expiration_day < state.current_day + state.delivery_lead_days) continue;
            const int quantity = std::min(remaining, batch.count - reserved[index]);
            reserved[index] += quantity;
            remaining -= quantity;
            supplied[item.first] += quantity;
            total_packages += quantity;
            subtotal += quantity * batch.batch_price * (1.0 + markup_percent / 100.0);
            acquisition_cost += quantity * ledger.batches[index].acquisition_price;
            if (remaining == 0) break;
        }
    }
    const double total = subtotal * (1.0 - state.discounts.percent_for(customers[customer_idx], subtotal) / 100.0);
    if (!std::isfinite(total) || !std::isfinite(acquisition_cost) ||
        !std::isfinite(ledger.sold_cost + acquisition_cost))
        throw std::overflow_error("Order amount overflow");
    for (auto count : supplied)
        if (count > std::numeric_limits<int>::max())
            throw std::overflow_error("Fulfilled quantity overflow");
    // Сначала проверяем весь заказ и рассчитываем выдачу, затем изменяем остатки.
    stats.update(total, 0.0, 0.0);
    for (std::size_t i = 0; i < reserved.size(); ++i)
        warehouse.inventory[i].take(reserved[i]);
    ledger.sold_cost += acquisition_cost;
    ledger.sold_packages += total_packages;
    for (std::size_t i = 0; i < supplied.size(); ++i)
        if (supplied[i] > 0) state.last_fulfillment.emplace_back(static_cast<int>(i), static_cast<int>(supplied[i]));
    final_price = total;
    return total_packages > 0;
}
