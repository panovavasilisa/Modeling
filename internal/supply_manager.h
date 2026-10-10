#pragma once
#include "back/data_generator.h"
#include "back/request.h"
#include "back/statistics.h"
#include "back/store.h"
#include <vector>

namespace model_detail {

struct PendingSupply {
    Request request;
    int ordered_day;
    int expected_day;
};

struct SupplyReceipt {
    int medicine_id;
    int count;
    int ordered_day;
    int arrived_day;
    double cost;
};

class SupplyManager {
public:
    SupplyManager(Warehouse& warehouse, Statistics& stats, DataGenerator& generator);
    int create_requests(int current_day);
    int advance_to(int current_day);
    const std::vector<PendingSupply>& pending() const { return pending_; }
    const std::vector<SupplyReceipt>& receipts() const { return receipts_; }
private:
    Warehouse& warehouse_;
    Statistics& stats_;
    DataGenerator& generator_;
    int last_processed_day_ = 0;
    std::vector<PendingSupply> pending_;
    std::vector<SupplyReceipt> receipts_;
};

} // namespace model_detail
