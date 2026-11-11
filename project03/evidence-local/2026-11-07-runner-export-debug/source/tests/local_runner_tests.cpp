#include "fixture.hpp"
#include "../examples/local_sim_scenario.hpp"
#include "p03/local_quote_runner.hpp"
#include "../examples/quote_runner_scenario.hpp"
using namespace fixture;
TEST_CASE(RUNNER_export_scenario_replays_all_five_frames_and_preserves_position_after_kill){
    const auto c=example::quote_runner_config();LocalQuoteRunner first(c),replay(c);
    CHECK(example::run_quote_scenario(first,c));CHECK_EQ(first.trace().size(),5U);
    for(const auto& frame:first.trace()){
        CHECK_EQ(replay.step(frame.input,frame.at),frame.error);
        CHECK_EQ(replay.trace().back().canonical,frame.canonical);
        CHECK_EQ(replay.trace().back().positions,frame.positions);
        CHECK_EQ(replay.trace().back().orders.size(),frame.orders.size());
    }
    CHECK_EQ(first.core().state().positions.booked()[0],1);CHECK_EQ(first.core().state().mode,CoreMode::Killed);
}
namespace {
LocalQuoteRunnerConfig runner_config(){
    auto c=config();c.trade_evidence_mode=TradeEvidenceMode::Simulator;
    const BusinessScope scope{c.anchor.account,c.contract.key(),c.anchor.day,c.anchor.epoch};
    return {c,example::simulator_config(c),{scope,c.strategy,EvidenceOrigin::LocalFixture,"fixture.book",1,10,1,10},128,4096};
}
MarketPulse pulse(const LocalQuoteRunnerConfig& c,std::uint64_t sequence=1,LogicalTime at=1,Ticks bid=499,Ticks ask=501){
    return {{c.strategy.scope,EvidenceOrigin::LocalFixture,"fixture.book",1,sequence,at,bid,ask,10,10,true},session()};
}
}
TEST_CASE(RUNNER_market_input_produces_recorded_mark_and_two_peer_accepted_orders){
    const auto c=runner_config();LocalQuoteRunner run(c);CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.trace().size(),1U);CHECK_EQ(run.trace()[0].ordinal,1U);CHECK_EQ(run.trace()[0].calls.size(),5U);
    CHECK(run.trace()[0].completed);
    CHECK(std::holds_alternative<ReferenceMark>(run.trace()[0].calls[0].input));CHECK_EQ(run.core().state().orders.size(),2U);
    CHECK_EQ(run.trace()[0].peer->orders.size(),2U);
    for(const auto& [id,order]:run.core().state().orders){(void)id;CHECK_EQ(order.execution(),ExecutionState::Working);}
}
TEST_CASE(RUNNER_price_update_receives_cancel_evidence_before_replacement_submits){
    const auto c=runner_config();LocalQuoteRunner run(c);CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.step(pulse(c,2,2,498,502),2),Error::None);
    const auto& calls=run.trace()[1].calls;std::size_t terminals=0,new_orders=0;
    for(const auto& call:calls){
        if(std::holds_alternative<TerminalReport>(call.input))++terminals;
        if(std::holds_alternative<SubmitOrder>(call.input)){CHECK_EQ(terminals,2U);++new_orders;}
    }
    CHECK_EQ(new_orders,2U);CHECK_EQ(run.core().state().orders.size(),4U);
    CHECK_EQ(run.core().state().orders.at({7,1}).parts().released,1);
    CHECK_EQ(run.core().state().orders.at({7,3}).intent().price,498);
}
TEST_CASE(RUNNER_invalid_market_withdraws_orders_and_kill_prevents_new_quotes){
    const auto c=runner_config();LocalQuoteRunner run(c);CHECK_EQ(run.step(pulse(c),1),Error::None);
    auto bad=pulse(c,2,2);bad.book.healthy=false;CHECK_EQ(run.step(bad,2),Error::Blocked);
    for(const auto& [id,order]:run.core().state().orders){(void)id;CHECK_EQ(order.parts().held(),0);}
    CHECK_EQ(run.step(pulse(c,3,3),3),Error::None);CHECK_EQ(run.step(StopLocalQuotes{},4),Error::None);
    CHECK_EQ(run.core().state().mode,CoreMode::Killed);
    CHECK_EQ(run.step(pulse(c,4,5),5),Error::Halted);
    for(const auto& [id,order]:run.core().state().orders){(void)id;CHECK_EQ(order.parts().held(),0);}
}
TEST_CASE(RUNNER_fill_records_actual_report_and_independent_peer_positions){
    const auto c=runner_config();LocalQuoteRunner run(c);CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.step(LocalFillInput{{7,1},"runner-fill-1",1,499},2),Error::None);
    CHECK_EQ(run.trace()[1].calls.size(),1U);CHECK_EQ(run.core().state().positions.booked()[0],1);
    CHECK_EQ(run.trace()[1].peer->positions,run.core().state().positions.booked());
    CHECK_EQ(run.step(LocalFillInput{{7,1},"runner-fill-1",1,499},3),Error::None);
    CHECK_EQ(run.trace()[2].calls[0].result.error,Error::Duplicate);CHECK_EQ(run.core().state().positions.booked()[0],1);
}
TEST_CASE(RUNNER_raw_input_replay_reproduces_core_peer_and_core_call_sequences){
    const auto c=runner_config();LocalQuoteRunner first(c),replay(c);
    CHECK_EQ(first.step(pulse(c),1),Error::None);CHECK_EQ(first.step(pulse(c,2,2,498,502),2),Error::None);
    CHECK_EQ(first.step(StopLocalQuotes{},3),Error::None);
    for(const auto& frame:first.trace())CHECK_EQ(replay.step(frame.input,frame.at),frame.error);
    CHECK_EQ(first.core().canonical_state(),replay.core().canonical_state());
    for(std::size_t i=0;i<first.trace().size();++i){
        const auto& a=first.trace()[i];const auto& b=replay.trace()[i];CHECK_EQ(a.canonical,b.canonical);
        CHECK_EQ(a.peer->revision,b.peer->revision);CHECK_EQ(a.peer->positions,b.peer->positions);CHECK_EQ(a.calls.size(),b.calls.size());
        for(std::size_t j=0;j<a.calls.size();++j){CHECK_EQ(a.calls[j].result.core_sequence,b.calls[j].result.core_sequence);CHECK_EQ(a.calls[j].result.error,b.calls[j].result.error);}
    }
}
TEST_CASE(RUNNER_bounds_and_out_of_order_input_do_not_create_unrecorded_core_effects){
    auto c=runner_config();c.max_inputs=1;LocalQuoteRunner run(c);CHECK_EQ(run.step(pulse(c),2),Error::None);
    const auto before=run.core().canonical_state();CHECK_EQ(run.step(pulse(c,2),1),Error::OutOfOrderTime);
    CHECK_EQ(run.trace().size(),1U);CHECK_EQ(run.core().canonical_state(),before);
    CHECK_EQ(run.step(pulse(c,2,3),3),Error::Capacity);CHECK(run.stopped());
    CHECK_EQ(run.core().canonical_state(),before);CHECK_EQ(run.step(StopLocalQuotes{},4),Error::Halted);
}
TEST_CASE(RUNNER_peer_budget_rejection_is_delivered_to_core_without_faking_acceptance){
    auto c=runner_config();c.peer.initial_budget=1;LocalQuoteRunner run(c);
    CHECK_EQ(run.step(pulse(c),1),Error::None);CHECK_EQ(run.core().state().orders.size(),2U);
    for(const auto& [id,order]:run.core().state().orders){(void)id;CHECK_EQ(order.execution(),ExecutionState::Rejected);CHECK_EQ(order.parts().released,1);}
    CHECK_EQ(run.trace()[0].peer->available_budget,1);CHECK(run.core().invariant());
}
TEST_CASE(RUNNER_partial_plan_risk_rejection_retains_first_accepted_order_and_its_reservation){
    auto c=runner_config();c.core.risk.local_funds=102;LocalQuoteRunner run(c);
    CHECK_EQ(run.step(pulse(c),1),Error::InsufficientFunds);CHECK(run.trace()[0].completed);
    CHECK_EQ(run.core().state().orders.size(),1U);CHECK_EQ(run.trace()[0].peer->orders.size(),1U);
    CHECK_EQ(run.core().state().orders.at({7,1}).parts().possible,1);
    CHECK_EQ(run.core().state().funds.view().local_required,102);
    CHECK_EQ(run.step(StopLocalQuotes{},2),Error::None);
    CHECK_EQ(run.core().state().orders.at({7,1}).parts().released,1);
}
TEST_CASE(RUNNER_invalid_baseline_and_core_call_budget_fail_closed){
    auto c=runner_config();c.peer.initial_positions[0]=1;CHECK_THROWS(LocalQuoteRunner(c),std::invalid_argument);
    c=runner_config();c.max_core_calls=4*c.core.capacity.orders+8;LocalQuoteRunner run(c);
    CHECK_EQ(run.step(pulse(c),1),Error::None);const auto before=run.core().canonical_state();
    CHECK_EQ(run.step(pulse(c,2,2),2),Error::Capacity);CHECK(run.stopped());
    CHECK_EQ(run.core().canonical_state(),before);CHECK_EQ(run.trace().size(),1U);
}
