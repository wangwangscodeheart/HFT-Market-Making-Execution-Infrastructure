#pragma once
#include "demo_scenario.hpp"
#include "p03/local_sim.hpp"

namespace p03::example {
// Shared reference/config inputs only. The peer never receives a CoreLoop or a
// PositionLedger, and performs its own rule and accounting decisions.
inline SimConfig simulator_config(const CoreConfig& core_config){
    const auto& c=core_config.contract;
    return {core_config.anchor.account,{c.instrument,c.exchange,c.day,c.low,c.high,c.tick_step,c.min_lots,c.max_lots,c.lot_step},
        core_config.anchor.booked,core_config.risk.local_funds,core_config.risk.opening_margin_per_lot,core_config.risk.fee_per_lot};
}
} // namespace p03::example
