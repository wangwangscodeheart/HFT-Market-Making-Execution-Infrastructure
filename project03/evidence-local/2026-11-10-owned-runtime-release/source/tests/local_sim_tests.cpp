#include "fixture.hpp"
#include "../examples/local_sim_scenario.hpp"
using namespace fixture;
namespace {
CoreConfig sim_core_config(){auto c=config();c.trade_evidence_mode=TradeEvidenceMode::Simulator;return c;}
SimOrderRequest peer_request(OrderId id={7,1},OrderIntent i=intent()){
    return {id,std::move(i),StrategyId{"fixture.default.strategy"},ClientOrderId{"fixture.client","request-1"}};
}
SimQuerySnapshot snapshot(const LocalSimVenue& venue,std::uint64_t id=1){
    const auto result=venue.query({7,id});CHECK_EQ(result.error,Error::None);CHECK(result.snapshot.has_value());
    return *result.snapshot;
}
void deliver(Fixture& f,const SimReply& reply){for(const auto& event:reply.reports)CHECK_EQ(f.apply(event).error,Error::None);}
}
TEST_CASE(LOCALSIM_query_is_a_detached_snapshot_of_independent_state) {
    LocalSimVenue venue(example::simulator_config(config()));auto q=snapshot(venue);
    CHECK_EQ(q.origin,EvidenceOrigin::LocalSimulator);CHECK_EQ(q.revision,0U);CHECK(q.orders.empty());
    q.positions[0]=999;q.available_budget=0;
    const auto fresh=snapshot(venue,2);CHECK_EQ(fresh.positions[0],0);CHECK_EQ(fresh.available_budget,1'000'000);
    CHECK_EQ(venue.query({}).error,Error::InvalidInput);CHECK(venue.invariant());
}
TEST_CASE(LOCALSIM_standard_A1a_fills_and_query_replay_match_actual_core) {
    auto scenario=example::standard_filled_order();scenario.config.trade_evidence_mode=TradeEvidenceMode::Simulator;
    Fixture f(scenario.config);LocalSimVenue venue(example::simulator_config(scenario.config));
    const auto& request=std::get<SubmitOrder>(scenario.inputs[1]);const auto submitted=f.apply(request);CHECK(submitted.order.has_value());
    const auto accepted=venue.receive(peer_request(*submitted.order,request.intent));CHECK_EQ(accepted.error,Error::None);deliver(f,accepted);
    for(std::size_t i=3;i<scenario.inputs.size();++i){
        const auto& source=std::get<TradeReport>(scenario.inputs[i]);const auto fill=venue.execute_fill(*submitted.order,source.trade.key.trade_id,source.trade.quantity,source.trade.price);
        CHECK_EQ(fill.error,Error::None);deliver(f,fill);CHECK(venue.invariant());
    }
    const auto q=snapshot(venue);CHECK_EQ(q.positions,f.core.state().positions.booked());CHECK_EQ(q.positions[0],10);
    CHECK_EQ(q.orders.at(0).filled,10);CHECK_EQ(q.orders.at(0).status,SimOrderStatus::Filled);CHECK_EQ(q.trades.size(),3U);
    CHECK_EQ(q.available_budget,998980);CHECK_EQ(f.order(*submitted.order).booked(),10);
    for(const auto& report:q.trades){CHECK_EQ(report.channel,Channel::Query);CHECK_EQ(report.stamp.origin,EvidenceOrigin::LocalSimulator);CHECK_EQ(f.apply(report).error,Error::Duplicate);}
    CHECK_EQ(f.core.state().positions.booked()[0],10);
}
TEST_CASE(LOCALSIM_A07_peer_fills_before_cancel_core_receives_trade_late) {
    Fixture f(sim_core_config());LocalSimVenue venue(example::simulator_config(sim_core_config()));const auto id=f.submit();
    deliver(f,venue.receive(peer_request(id)));deliver(f,venue.execute_fill(id,"t1",1,499));
    const auto delayed=venue.execute_fill(id,"t2",1,499);CHECK_EQ(delayed.error,Error::None);
    CHECK_EQ(snapshot(venue).positions[0],2);CHECK_EQ(f.core.state().positions.booked()[0],1);
    const auto cancel=f.apply(RequestCancel{id});CHECK(cancel.cancel.has_value());deliver(f,venue.cancel(id,*cancel.cancel));
    CHECK_EQ(f.order(id).parts(),(QuantityParts{1,1,0,1,0}));CHECK_EQ(f.core.state().funds.view().local_required,204);
    deliver(f,delayed);CHECK_EQ(f.order(id).parts(),(QuantityParts{2,0,0,1,0}));
    const auto q=snapshot(venue);CHECK_EQ(q.positions,f.core.state().positions.booked());CHECK_EQ(q.available_budget,999796);
    for(const auto& report:q.trades)CHECK_EQ(f.apply(report).error,Error::Duplicate);
    CHECK_EQ(f.core.state().funds.view().local_required,204);CHECK(venue.invariant());
}
TEST_CASE(LOCALSIM_peer_rule_or_budget_rejection_is_independent_of_core_approval) {
    for(int variant=0;variant<2;++variant){
        auto scenario=example::standard_filled_order();scenario.config.trade_evidence_mode=TradeEvidenceMode::Simulator;
        Fixture f(scenario.config);auto peer=example::simulator_config(scenario.config);
        if(variant==0)peer.rules.max_lots=5;else peer.initial_budget=100;
        LocalSimVenue venue(peer);const auto& input=std::get<SubmitOrder>(scenario.inputs[1]);const auto accepted=f.apply(input);CHECK(accepted.order.has_value());
        const auto rejected=venue.receive(peer_request(*accepted.order,input.intent));
        CHECK_EQ(rejected.error,variant==0?Error::QuantityRule:Error::InsufficientFunds);CHECK_EQ(rejected.reports.size(),1U);deliver(f,rejected);
        CHECK_EQ(f.order(*accepted.order).execution(),ExecutionState::Rejected);CHECK_EQ(f.order(*accepted.order).parts().released,10);
        CHECK_EQ(f.core.state().funds.view().local_required,0);CHECK_EQ(snapshot(venue).positions[0],0);
    }
}
TEST_CASE(LOCALSIM_six_position_effects_use_peer_accounting_not_core_position_ledger) {
    const std::array<std::pair<Side,Offset>,6> actions{{{Side::Buy,Offset::Open},{Side::Sell,Offset::Open},
        {Side::Sell,Offset::CloseToday},{Side::Sell,Offset::CloseYesterday},{Side::Buy,Offset::CloseToday},{Side::Buy,Offset::CloseYesterday}}};
    const std::array<std::array<Lots,4>,6> expected{{{6,6,7,8},{5,6,8,8},{4,6,7,8},{5,5,7,8},{5,6,6,8},{5,6,7,7}}};
    for(std::size_t i=0;i<actions.size();++i){LocalSimVenue venue(example::simulator_config(config({5,6,7,8})));
        const auto req=peer_request({7,1},intent(actions[i].first,actions[i].second,1,499));
        CHECK_EQ(venue.receive(req).error,Error::None);CHECK_EQ(venue.execute_fill(req.order,"fill-1",1,499).error,Error::None);
        const auto q=snapshot(venue);CHECK_EQ(q.positions,expected[i]);CHECK_EQ(q.frozen,(std::array<Lots,4>{}));CHECK(venue.invariant());
    }
}
TEST_CASE(LOCALSIM_duplicate_requests_and_fills_do_not_create_extra_peer_exposure) {
    LocalSimVenue venue(example::simulator_config(config()));const auto req=peer_request();
    CHECK_EQ(venue.receive(req).error,Error::None);const auto before=snapshot(venue);
    CHECK_EQ(venue.receive(req).error,Error::Duplicate);CHECK_EQ(snapshot(venue).revision,before.revision);
    auto changed=req;changed.intent.quantity=2;CHECK_EQ(venue.receive(changed).error,Error::EvidenceConflict);
    CHECK_EQ(venue.execute_fill(req.order,"t1",1,499).error,Error::None);const auto after=snapshot(venue);
    const auto duplicate=venue.execute_fill(req.order,"t1",1,499);CHECK_EQ(duplicate.error,Error::Duplicate);CHECK_EQ(duplicate.reports.size(),1U);
    CHECK_EQ(venue.execute_fill(req.order,"t1",2,499).error,Error::EvidenceConflict);
    const auto final=snapshot(venue);CHECK_EQ(final.positions,after.positions);CHECK_EQ(final.available_budget,after.available_budget);CHECK_EQ(final.trades.size(),1U);
}
TEST_CASE(LOCALSIM_invalid_fill_and_close_shortage_leave_peer_state_unchanged) {
    LocalSimVenue venue(example::simulator_config(config()));const auto req=peer_request();CHECK_EQ(venue.receive(req).error,Error::None);
    const auto before=snapshot(venue);
    CHECK_EQ(venue.execute_fill(req.order,"overfill",4,499).error,Error::QuantityRule);
    CHECK_EQ(venue.execute_fill(req.order,"worse-price",1,500).error,Error::PriceRule);
    CHECK_EQ(venue.execute_fill(req.order,"",1,499).error,Error::InvalidInput);
    const auto after=snapshot(venue);CHECK_EQ(after.revision,before.revision);CHECK_EQ(after.available_budget,before.available_budget);CHECK_EQ(after.positions,before.positions);
    const auto close=venue.receive(peer_request({7,2},intent(Side::Sell,Offset::CloseToday,1,501)));
    CHECK_EQ(close.error,Error::InsufficientClose);CHECK_EQ(snapshot(venue).frozen,(std::array<Lots,4>{}));
}
TEST_CASE(LOCALSIM_capacity_and_duplicate_cancel_preserve_resources) {
    auto cfg=example::simulator_config(config());cfg.max_orders=1;cfg.max_trades=1;cfg.max_cancels=1;LocalSimVenue venue(cfg);
    const auto req=peer_request();CHECK_EQ(venue.receive(req).error,Error::None);
    CHECK_EQ(venue.receive(peer_request({7,2})).error,Error::Capacity);
    CHECK_EQ(venue.execute_fill(req.order,"t1",1,499).error,Error::None);
    CHECK_EQ(venue.execute_fill(req.order,"t2",1,499).error,Error::Capacity);
    CHECK_EQ(venue.cancel(req.order,{7,1}).error,Error::None);const auto before=snapshot(venue);
    CHECK_EQ(venue.cancel(req.order,{7,1}).error,Error::Duplicate);const auto after=snapshot(venue);
    CHECK_EQ(after.available_budget,before.available_budget);CHECK_EQ(after.available_budget,999898);
    CHECK_EQ(after.orders.at(0).status,SimOrderStatus::Canceled);CHECK_EQ(venue.cancel(req.order,{7,2}).error,Error::Capacity);
}
TEST_CASE(LOCALSIM_backend_origin_profiles_cannot_be_mixed) {
    Fixture fixture_core;const auto fixture_id=fixture_core.submit();LocalSimVenue venue(example::simulator_config(config()));
    const auto accepted=venue.receive(peer_request(fixture_id));CHECK_EQ(fixture_core.apply(accepted.reports.at(0)).error,Error::UntrustedEvidence);
    Fixture sim_core(sim_core_config());const auto sim_id=sim_core.submit();CHECK_EQ(sim_core.apply(WorkingReport{header(sim_id)}).error,Error::UntrustedEvidence);
    CHECK_EQ(sim_core.core.state().positions.booked()[0],0);
    CHECK_EQ(sim_core.apply(ObservePosition{position_observation()}).error,Error::Blocked);
}
TEST_CASE(LOCALSIM_invalid_configuration_and_profile_fail_closed) {
    auto cfg=example::simulator_config(config());cfg.initial_budget=-1;CHECK_THROWS(LocalSimVenue{cfg},std::invalid_argument);
    cfg=example::simulator_config(config());cfg.max_orders=0;CHECK_THROWS(LocalSimVenue{cfg},std::invalid_argument);
    auto core=config();core.trade_evidence_mode=static_cast<TradeEvidenceMode>(99);CHECK_THROWS(CoreLoop{core},std::invalid_argument);
    CHECK_EQ(trade_origin(static_cast<TradeEvidenceMode>(99)),EvidenceOrigin::Unverified);
}
