#pragma once
#include "local_sim_scenario.hpp"
#include "p03/local_delivery.hpp"
#include "p03/local_query_review.hpp"
#include <algorithm>

namespace p03::example {
struct ReplayFrame {
    std::string action;
    LogicalTime at{};
    std::uint64_t peer_revision{},report_id{},ingress{},sequence{};
    Error error{Error::None};
    std::array<Lots,4> peer_positions{},core_positions{};
    QuantityParts parts{};
    bool passed{};
};
struct ReplayResult {std::vector<ReplayFrame> frames;bool passed{};std::size_t blocked_queries{};};
// Fixed deterministic recovery mechanism experiment, not a historical incident trace.
inline ReplayResult delayed_cancel_replay(bool query_first=false){
    auto source=standard_filled_order();source.config.trade_evidence_mode=TradeEvidenceMode::Simulator;
    CoreLoop core(source.config);LocalSimVenue peer(simulator_config(source.config));LocalReportDelivery link;
    ReplayResult run;LogicalTime clock=0;OrderId order{};
    auto expect=[](bool ok){if(!ok)throw std::logic_error("local replay prerequisite failed");};
    expect(core.step(source.inputs[0],clock++).error==Error::None);
    const auto& input=std::get<SubmitOrder>(source.inputs[1]);const auto submit=core.step(input,clock++);
    expect(submit.error==Error::None && submit.order.has_value());order=*submit.order;
    auto observe=[&](std::string action,StepResult result,std::uint64_t report,Lots peer_lots,Lots booked,Lots suspense,Lots released,Error error){
        const auto q=peer.query({7,static_cast<std::uint64_t>(run.frames.size()+1)});expect(q.snapshot.has_value());
        const auto p=core.state().orders.at(order).parts();
        const bool passed=result.error==error && q.snapshot->positions==std::array<Lots,4>{peer_lots,0,0,0} &&
            core.state().positions.booked()==std::array<Lots,4>{booked,0,0,0} &&
            p.booked==booked && p.suspense==suspense && p.released==released && p.unresolved==0 &&
            p.possible==10-booked-suspense-released && core.invariant() && peer.invariant();
        run.frames.push_back({std::move(action),clock-1,q.snapshot->revision,report,result.ingress_sequence,result.core_sequence,
            result.error,q.snapshot->positions,core.state().positions.booked(),p,passed});
    };
    expect(link.capture(peer.receive({order,input.intent,source.config.strategy,input.client_id}))==Error::None);
    observe("peer_accepts",{},0,0,0,0,0,Error::None);
    auto delivered=link.deliver(core,1,clock++);observe("deliver_working",delivered,1,0,0,0,0,Error::None);
    const auto& trade=std::get<TradeReport>(source.inputs[3]).trade;
    expect(link.capture(peer.execute_fill(order,trade.key.trade_id,trade.quantity,trade.price))==Error::None);
    observe("peer_fills_report_held",{},0,2,0,0,0,Error::None);
    const auto cancel=core.step(RequestCancel{order},clock++);expect(cancel.cancel.has_value());
    observe("request_cancel",cancel,0,2,0,0,0,Error::None);
    expect(link.capture(peer.cancel(order,*cancel.cancel))==Error::None);
    observe("peer_cancels_remainder",{},0,2,0,0,0,Error::None);
    delivered=link.deliver(core,3,clock++);observe("deliver_cancel_reply",delivered,3,2,0,0,0,Error::None);
    delivered=link.deliver(core,4,clock++);observe("deliver_terminal_before_trade",delivered,4,2,0,2,8,Error::None);
    const auto query=peer.query({7,100});expect(query.snapshot.has_value() && query.snapshot->trades.size()==1);
    if(query_first){
        auto partial=*query.snapshot;partial.trades.clear();
        const auto before=core.canonical_state();
        expect(review_local_query(partial,simulator_config(source.config)).status==LocalQueryStatus::Incomplete);
        expect(before==core.canonical_state());++run.blocked_queries;
        expect(review_local_query(*query.snapshot,simulator_config(source.config)).status==LocalQueryStatus::Complete);
        const auto recovered=core.step(query.snapshot->trades[0],clock++);
        observe("query_recovers_missing_trade",recovered,0,2,2,0,8,Error::None);
        delivered=link.deliver(core,2,clock++);observe("late_push_after_query",delivered,2,2,2,0,8,Error::Duplicate);
        const auto repeated=core.step(query.snapshot->trades[0],clock++);
        observe("repeat_query_trade",repeated,0,2,2,0,8,Error::Duplicate);
    }else{
        delivered=link.deliver(core,2,clock++);observe("deliver_delayed_trade",delivered,2,2,2,0,8,Error::None);
        delivered=link.deliver(core,2,clock++);observe("repeat_same_delivery",delivered,2,2,2,0,8,Error::Duplicate);
        const auto replay=core.step(query.snapshot->trades[0],clock++);
        observe("query_trade_replay",replay,0,2,2,0,8,Error::Duplicate);
    }
    run.passed=std::all_of(run.frames.begin(),run.frames.end(),[](const auto& f){return f.passed;});
    return run;
}
} // namespace p03::example
