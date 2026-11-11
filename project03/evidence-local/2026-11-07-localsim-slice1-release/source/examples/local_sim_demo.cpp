#include "local_sim_scenario.hpp"
#include <iostream>

int main(){
    using namespace p03;
    auto scenario=example::standard_filled_order();scenario.config.trade_evidence_mode=TradeEvidenceMode::Simulator;
    CoreLoop core(scenario.config);LocalSimVenue venue(example::simulator_config(scenario.config));
    LogicalTime time=0;
    if(core.step(scenario.inputs[0],time++).error!=Error::None)return 1;
    const auto& input=std::get<SubmitOrder>(scenario.inputs[1]);const auto submitted=core.step(input,time++);
    if(submitted.error!=Error::None || !submitted.applied || !submitted.order)return 1;
    const auto accepted=venue.receive({*submitted.order,input.intent,scenario.config.strategy,input.client_id});
    if(accepted.error!=Error::None)return 1;
    for(const auto& report:accepted.reports)if(core.step(report,time++).error!=Error::None)return 1;
    std::cout<<"LOCAL_SIM deterministic peer; FIX A.1.a quantities scaled 1:1000; no exchange connection.\n";
    for(std::size_t i=3;i<scenario.inputs.size();++i){
        const auto& source=std::get<TradeReport>(scenario.inputs[i]);
        const auto filled=venue.execute_fill(*submitted.order,source.trade.key.trade_id,source.trade.quantity,source.trade.price);
        if(filled.error!=Error::None)return 1;
        for(const auto& report:filled.reports)if(core.step(report,time++).error!=Error::None)return 1;
        const auto query=venue.query({7,static_cast<std::uint64_t>(i)});
        if(!query.snapshot)return 1;
        std::cout<<"sim_revision="<<query.snapshot->revision<<" peer_long_today="<<query.snapshot->positions[0]
            <<" core_long_today="<<core.state().positions.booked()[0]<<" budget="<<query.snapshot->available_budget<<'\n';
    }
    const auto query=venue.query({7,10});if(!query.snapshot)return 1;
    for(const auto& report:query.snapshot->trades)if(core.step(report,time++).error!=Error::Duplicate)return 1;
    const bool matched=query.snapshot->positions==core.state().positions.booked() &&
        query.snapshot->orders.at(0).filled==core.state().orders.at(*submitted.order).booked();
    std::cout<<"FINAL peer_core_match="<<matched<<" fills="<<query.snapshot->trades.size()
        <<" booked="<<core.state().orders.at(*submitted.order).booked()<<" origin=LocalSimulator\n";
    return matched && core.invariant() && venue.invariant()?0:1;
}
