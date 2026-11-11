#include "fixture.hpp"
#include "p03/quote_strategy.hpp"
#include "../examples/local_sim_scenario.hpp"
#include "p03/local_query_review.hpp"
using namespace fixture;
namespace {
QuoteStrategyConfig strategy_config(const CoreLoop& core){return {core.scope(),core.config().strategy,EvidenceOrigin::LocalFixture,"fixture.book",1,10,1,10};}
MarketTop book(const CoreLoop& core){return {core.scope(),EvidenceOrigin::LocalFixture,"fixture.book",1,1,2,499,501,10,10,true};}
}
TEST_CASE(STRATEGY_valid_local_top_generates_passive_targets_without_mutating_core){
    Fixture f;QuoteStrategy strategy(strategy_config(f.core));const auto before=f.core.canonical_state();
    const auto decision=strategy.evaluate(f.core,book(f.core),session(),2,1);
    CHECK_EQ(decision.error,Error::None);CHECK(!decision.withdraw);CHECK(decision.target.has_value());
    CHECK_EQ(decision.target->buy->price,499);CHECK_EQ(decision.target->sell->price,501);
    CHECK_EQ(f.core.canonical_state(),before);
    const auto plan=plan_quotes(f.core,*decision.target,1);CHECK_EQ(plan.intents.size(),2U);
    for(const auto& i:plan.intents)CHECK_EQ(f.apply(std::get<SubmitOrder>(i)).error,Error::None);
}
TEST_CASE(STRATEGY_duplicate_sequence_old_generation_and_wrong_epoch_do_not_requote){
    Fixture f;QuoteStrategy strategy(strategy_config(f.core));auto top=book(f.core);
    CHECK_EQ(strategy.evaluate(f.core,top,session(),2,1).error,Error::None);
    CHECK_EQ(strategy.evaluate(f.core,top,session(),3,2).error,Error::Duplicate);
    ++top.sequence;CHECK_EQ(strategy.evaluate(f.core,top,session(),3,1).error,Error::Blocked);
    ++top.epoch;CHECK_EQ(strategy.evaluate(f.core,top,session(),3,2).error,Error::UntrustedEvidence);
}
TEST_CASE(STRATEGY_stale_future_unhealthy_crossed_and_closed_session_request_withdrawal){
    Fixture f;
    for(int fault=0;fault<5;++fault){
        QuoteStrategy strategy(strategy_config(f.core));auto top=book(f.core);auto s=session();LogicalTime now=2;
        if(fault==0)now=13;if(fault==1)top.received_at=3;if(fault==2)top.healthy=false;
        if(fault==3)top.ask=top.bid;if(fault==4)s.second=12*3600;
        const auto result=strategy.evaluate(f.core,top,s,now,1);CHECK(result.error!=Error::None);CHECK(result.withdraw);CHECK(!result.target);
    }
}
TEST_CASE(STRATEGY_inventory_limit_suppresses_risk_increasing_side_and_routes_reduction){
    Fixture f(config({10,0,0,0}));QuoteStrategy strategy(strategy_config(f.core));
    const auto decision=strategy.evaluate(f.core,book(f.core),session(),2,1);
    CHECK_EQ(decision.error,Error::None);CHECK(!decision.target->buy);CHECK(decision.target->sell.has_value());
    const auto plan=plan_quotes(f.core,*decision.target,1);CHECK_EQ(plan.intents.size(),1U);
    CHECK_EQ(std::get<SubmitOrder>(plan.intents[0]).intent.offset,Offset::CloseToday);
}
TEST_CASE(STRATEGY_kill_and_missing_trade_evidence_prevent_new_targets){
    Fixture f;QuoteStrategy strategy(strategy_config(f.core));CHECK_EQ(f.apply(KillRequested{}).error,Error::None);
    CHECK_EQ(strategy.evaluate(f.core,book(f.core),session(),2,1).error,Error::Halted);
    Fixture missing;const auto id=missing.submit();CHECK_EQ(missing.apply(TerminalReport{header(id),{TerminalKind::Canceled,1}}).error,Error::None);
    QuoteStrategy stopped(strategy_config(missing.core));CHECK_EQ(stopped.evaluate(missing.core,book(missing.core),session(),3,1).error,Error::Blocked);
}
TEST_CASE(STRATEGY_targets_planner_core_and_independent_peer_complete_a_local_cycle){
    auto c=config();c.trade_evidence_mode=TradeEvidenceMode::Simulator;Fixture f(c);
    LocalSimVenue peer(example::simulator_config(c));QuoteStrategy strategy(strategy_config(f.core));
    const auto decision=strategy.evaluate(f.core,book(f.core),session(),2,1);CHECK(decision.target.has_value());
    const auto plan=plan_quotes(f.core,*decision.target,1);CHECK_EQ(plan.intents.size(),2U);
    std::vector<OrderId> ids;
    for(const auto& item:plan.intents){
        const auto& submit=std::get<SubmitOrder>(item);const auto result=f.apply(submit);CHECK(result.order.has_value());ids.push_back(*result.order);
        const auto accepted=peer.receive({*result.order,submit.intent,c.strategy,submit.client_id});CHECK_EQ(accepted.error,Error::None);
        for(const auto& report:accepted.reports)CHECK_EQ(f.apply(report).error,Error::None);
    }
    const auto fill=peer.execute_fill(ids[0],"strategy-cycle-1",1,499);CHECK_EQ(fill.error,Error::None);
    for(const auto& report:fill.reports)CHECK_EQ(f.apply(report).error,Error::None);
    const auto cancel=f.apply(RequestCancel{ids[1]});CHECK(cancel.cancel.has_value());
    const auto canceled=peer.cancel(ids[1],*cancel.cancel);CHECK_EQ(canceled.error,Error::None);
    for(const auto& report:canceled.reports)CHECK_EQ(f.apply(report).error,Error::None);
    const auto q=peer.query({7,1});CHECK(q.snapshot.has_value());
    CHECK_EQ(review_local_query(*q.snapshot,example::simulator_config(c)).status,LocalQueryStatus::Complete);
    CHECK_EQ(q.snapshot->positions,f.core.state().positions.booked());CHECK_EQ(q.snapshot->positions[0],1);
    CHECK(peer.invariant());CHECK(f.core.invariant());
}
TEST_CASE(STRATEGY_near_inventory_limits_caps_quantity_on_both_sides){
    for(const bool short_side:{false,true}){
        Fixture f(config(short_side?std::array<Lots,4>{0,0,9,0}:std::array<Lots,4>{9,0,0,0}));
        auto c=strategy_config(f.core);c.quantity=3;QuoteStrategy strategy(c);
        const auto before=f.core.canonical_state();const auto d=strategy.evaluate(f.core,book(f.core),session(),2,1);
        CHECK_EQ(d.error,Error::None);CHECK(d.target.has_value());CHECK(d.target->buy);CHECK(d.target->sell);
        CHECK_EQ(d.target->buy->quantity,short_side?3:1);CHECK_EQ(d.target->sell->quantity,short_side?1:3);
        CHECK_EQ(f.core.canonical_state(),before);
        const auto plan=plan_quotes(f.core,*d.target,1);CHECK_EQ(plan.error,Error::None);CHECK_EQ(plan.intents.size(),2U);
        for(const auto& item:plan.intents){
            const auto& order=std::get<SubmitOrder>(item).intent;
            CHECK_EQ(f.core.config().contract.validate(order),Error::None);
            const Lots net=short_side?-9:9;
            const auto after=net+(order.side==Side::Buy?order.quantity:-order.quantity);
            CHECK(after>=-10 && after<=10);
        }
    }
}
TEST_CASE(STRATEGY_inventory_headroom_rounds_down_to_valid_lots_or_suppresses_side){
    for(const Lots initial:{7,9}){
        auto core_config=config({initial,0,0,0});core_config.contract.min_lots=2;core_config.contract.lot_step=2;
        Fixture f(core_config);auto c=strategy_config(f.core);c.quantity=4;QuoteStrategy strategy(c);
        const auto d=strategy.evaluate(f.core,book(f.core),session(),2,1);
        CHECK_EQ(d.error,Error::None);CHECK(d.target.has_value());CHECK(d.target->sell.has_value());
        CHECK_EQ(d.target->sell->quantity,4);
        if(initial==7){CHECK(d.target->buy.has_value());CHECK_EQ(d.target->buy->quantity,2);}
        else CHECK(!d.target->buy.has_value());
    }
}
