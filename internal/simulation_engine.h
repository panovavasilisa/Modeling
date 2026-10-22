#pragma once
#include "back/data_generator.h"
#include "back/order.h"
#include "back/simulation.h"
#include "supply_manager.h"
#include <deque>
#include <iosfwd>
#include <memory>

namespace model_detail {

struct QueuedOrder {
    std::uint64_t id;
    int ordered_day;
    Order order;
};

// Управляет взаимодействием Warehouse, Order, Customer, Request, DataGenerator
// и Statistics. Курьеры представлены индексами массива нагрузки размера M.
class SimulationEngine {
public:
    void configure(const SimulationParameters& parameters);
    void initialize();
    void begin_day(int day);
    void generate_events(int day);
    void process_orders(int day);
    void finish_day();
    const SimulationParameters& parameters() const { return parameters_; }
    const SimulationResult& result() const { return result_; }
    void write_summary(std::ostream& output) const;
private:
    enum class Phase { uninitialized, finished, begun, generated, processed };
    SimulationParameters parameters_;
    SimulationResult result_;
    Statistics stats_{};
    DataGenerator generator_;
    std::unique_ptr<SupplyManager> supplier_;
    std::deque<QueuedOrder> orders_;
    std::vector<CompletedPurchase> tomorrow_;
    DailySimulationResult today_;
    Phase phase_ = Phase::uninitialized;
    int day_ = 0;
    std::uint64_t next_id_ = 1;
    double delivered_cost_ = 0.0;
    void require_phase(Phase phase, int day) const;
    void load_initial_stock();
    void update_result();
};

SimulationEngine& engine();

} // пространство имён model_detail
