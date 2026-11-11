#include "fixture.hpp"
#include "../examples/local_sim_scenario.hpp"
#include "p03/local_quote_runner.hpp"
#include "../examples/quote_runner_scenario.hpp"
using namespace fixture;
TEST_CASE(RUNNER_late_fill_after_kill_replay_keeps_suspense_until_actual_trade_and_repeat_stop_is_safe){
    const auto c=example::quote_late_fill_config();LocalQuoteRunner first(c),replay(c);
    CHECK(example::run_quote_late_fill_scenario(first,c));CHECK_EQ(first.trace().size(),7U);
    for(const auto& f:first.trace()){
        CHECK_EQ(replay.step(f.input,f.at),f.error);CHECK_EQ(replay.trace().back().canonical,f.canonical);
        CHECK_EQ(replay.trace().back().orders[0].parts,f.orders[0].parts);CHECK_EQ(replay.trace().back().cleanup_lane,f.cleanup_lane);
    }
    const auto before=first.core().canonical_state();
    CHECK_EQ(first.step(example::runner_pulse(c,2,8,499,501),8),Error::Halted);CHECK_EQ(first.core().canonical_state(),before);
}
TEST_CASE(RUNNER_query_can_fill_suspense_after_kill_before_original_push_and_cannot_reactivate){
    const auto c=example::quote_late_fill_config();LocalQuoteRunner run(c);
    CHECK_EQ(run.step(example::runner_pulse(c,1,1,499,501),1),Error::None);
    CHECK_EQ(run.step(LocalDeferredFillInput{{{7,1},"query-after-kill",1,499}},2),Error::None);
    CHECK_EQ(run.step(StopLocalQuotes{},3),Error::None);
    CHECK_EQ(run.trace().back().orders[0].parts,(QuantityParts{0,1,0,2,0}));
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,1},false},4),Error::None);
    CHECK_EQ(run.trace().back().orders[0].parts,(QuantityParts{1,0,0,2,0}));
    CHECK_EQ(run.step(LocalHeldReportInput{0},5),Error::Duplicate);
    CHECK_EQ(run.core().state().mode,CoreMode::Killed);CHECK_EQ(run.trace().back().core_required,102);
    CHECK_EQ(run.step(StopLocalQuotes{},6),Error::None);CHECK_EQ(run.trace().back().calls.size(),1U);
    CHECK_EQ(run.core().state().positions.booked()[0],1);
}
TEST_CASE(RUNNER_cancel_rejection_for_already_filled_peer_does_not_prove_core_order_was_unfilled){
    auto c=example::quote_late_fill_config();c.strategy.quantity=1;LocalQuoteRunner run(c);
    CHECK_EQ(run.step(example::runner_pulse(c,1,1,499,501),1),Error::None);
    CHECK_EQ(run.step(LocalDeferredFillInput{{{7,1},"full-before-kill",1,499}},2),Error::None);
    CHECK_EQ(run.step(StopLocalQuotes{},3),Error::None);
    CHECK_EQ(run.trace().back().orders[0].parts,(QuantityParts{0,0,1,0,0}));
    CHECK_EQ(run.trace().back().core_required,102);
    CHECK_EQ(run.step(LocalHeldReportInput{0},4),Error::None);
    CHECK_EQ(run.trace().back().orders[0].parts,(QuantityParts{1,0,0,0,0}));
    CHECK_EQ(run.core().state().mode,CoreMode::Killed);CHECK_EQ(run.trace().back().peer->positions[0],1);
}
TEST_CASE(RUNNER_cleanup_export_includes_rejected_admission_checks_and_replays_full_experiment){
    const auto c=example::quote_cleanup_config();LocalQuoteRunner first(c),replay(c);
    std::vector<example::RunnerAdmissionCheck> checks;
    CHECK(example::run_quote_cleanup_scenario(first,c,&checks));CHECK(example::run_quote_cleanup_scenario(replay,c));
    CHECK_EQ(checks.size(),3U);for(const auto& check:checks){CHECK_EQ(check.actual,check.expected);CHECK_EQ(check.frames_before,check.frames_after);CHECK(check.core_unchanged);}
    CHECK_EQ(first.trace().size(),6U);
    for(std::size_t i=0;i<first.trace().size();++i){CHECK_EQ(first.trace()[i].canonical,replay.trace()[i].canonical);CHECK_EQ(first.trace()[i].cleanup_lane,replay.trace()[i].cleanup_lane);}
    CHECK(first.stopped());CHECK(first.cleanup_exhausted());CHECK(replay.cleanup_exhausted());
}
TEST_CASE(RUNNER_overflow_export_replay_preserves_loss_after_queue_and_query_success){
    const auto c=example::quote_overflow_config();LocalQuoteRunner first(c),replay(c);
    CHECK(example::run_quote_overflow_scenario(first,c));CHECK_EQ(first.trace().size(),10U);
    for(const auto& f:first.trace()){
        CHECK_EQ(replay.step(f.input,f.at),f.error);const auto& r=replay.trace().back();
        CHECK_EQ(r.canonical,f.canonical);CHECK_EQ(r.pending_delivery,f.pending_delivery);CHECK_EQ(r.delivery_loss,f.delivery_loss);
        CHECK_EQ(r.core_required,f.core_required);CHECK_EQ(r.peer->available_budget,f.peer->available_budget);
    }
}
TEST_CASE(RUNNER_retry_export_replay_preserves_failed_batch_and_query_stop_after_draining){
    const auto c=example::quote_runner_config();LocalQuoteRunner first(c),replay(c);
    CHECK(example::run_quote_retry_scenario(first,c));CHECK_EQ(first.trace().size(),10U);
    for(const auto& f:first.trace()){
        CHECK_EQ(replay.step(f.input,f.at),f.error);const auto& r=replay.trace().back();
        CHECK_EQ(r.canonical,f.canonical);CHECK_EQ(r.pending_delivery,f.pending_delivery);CHECK_EQ(r.pending_reports,f.pending_reports);
        CHECK_EQ(r.query_blocked,f.query_blocked);CHECK_EQ(r.calls.size(),f.calls.size());
    }
    CHECK_EQ(first.trace().back().positions,(std::array<Lots,4>{1,0,1,0}));
}
TEST_CASE(RUNNER_query_gap_export_replay_preserves_pending_and_rejected_query_without_core_changes){
    const auto c=example::quote_runner_config();LocalQuoteRunner first(c),replay(c);
    CHECK(example::run_quote_query_scenario(first,c));CHECK_EQ(first.trace().size(),8U);
    for(const auto& frame:first.trace()){
        CHECK_EQ(replay.step(frame.input,frame.at),frame.error);const auto& r=replay.trace().back();
        CHECK_EQ(r.canonical,frame.canonical);CHECK_EQ(r.pending_reports,frame.pending_reports);
        CHECK_EQ(r.peer->positions,frame.peer->positions);CHECK_EQ(r.calls.size(),frame.calls.size());
    }
}
TEST_CASE(RUNNER_expiry_export_replay_preserves_noop_boundary_and_new_quote_recovery){
    const auto c=example::quote_runner_config();LocalQuoteRunner first(c),replay(c);
    CHECK(example::run_quote_expiry_scenario(first,c));CHECK_EQ(first.trace().size(),6U);
    for(const auto& f:first.trace()){
        CHECK_EQ(replay.step(f.input,f.at),f.error);CHECK_EQ(replay.trace().back().canonical,f.canonical);
        CHECK_EQ(replay.trace().back().calls.size(),f.calls.size());
    }
    CHECK_EQ(first.trace()[1].canonical,first.trace()[0].canonical);
    CHECK_EQ(first.trace()[3].canonical,first.trace()[2].canonical);
}
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
    CHECK_EQ(run.core().canonical_state(),before);CHECK_EQ(run.step(StopLocalQuotes{},4),Error::None);
    CHECK(run.trace().back().cleanup_lane);CHECK_EQ(run.core().state().mode,CoreMode::Killed);
    for(const auto& order:run.trace().back().orders)CHECK_EQ(order.parts.held(),0);
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
    c=runner_config();c.max_core_calls=4*c.core.capacity.orders+c.peer.max_trades+8;LocalQuoteRunner run(c);
    CHECK_EQ(run.step(pulse(c),1),Error::None);const auto before=run.core().canonical_state();
    CHECK_EQ(run.step(pulse(c,2,2),2),Error::Capacity);CHECK(run.stopped());
    CHECK_EQ(run.core().canonical_state(),before);CHECK_EQ(run.trace().size(),1U);
    CHECK_EQ(run.step(StopLocalQuotes{},3),Error::None);CHECK(run.trace().back().cleanup_lane);
}
TEST_CASE(RUNNER_cleanup_reserve_retries_failed_receipt_then_stops_after_normal_input_quota){
    auto c=runner_config();c.max_inputs=3;c.cleanup_inputs=2;LocalQuoteRunner run(c);
    CHECK_EQ(run.step(pulse(c),1),Error::None);CHECK_EQ(run.step(LocalReportFailureInput{0},2),Error::None);
    CHECK_EQ(run.step(LocalFillInput{{7,1},"cleanup-fill",1,499},3),Error::InjectedFailure);
    CHECK_EQ(run.trace().back().pending_delivery,1U);
    CHECK_EQ(run.step(pulse(c,2,4),4),Error::Capacity);CHECK(run.stopped());
    CHECK_EQ(run.step(LocalRetryEvidenceInput{},5),Error::None);CHECK(run.trace().back().cleanup_lane);
    CHECK_EQ(run.core().state().positions.booked()[0],1);CHECK_EQ(run.trace().back().pending_delivery,0U);
    CHECK_EQ(run.step(StopLocalQuotes{},6),Error::None);CHECK_EQ(run.core().state().mode,CoreMode::Killed);
    const auto before=run.core().canonical_state();const auto frames=run.trace().size();
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,1},false},7),Error::Capacity);CHECK(run.cleanup_exhausted());
    CHECK_EQ(run.core().canonical_state(),before);CHECK_EQ(run.trace().size(),frames);
    CHECK_EQ(run.step(pulse(c,3,8),8),Error::Halted);
}
TEST_CASE(RUNNER_cleanup_core_call_budget_is_separate_bounded_and_cannot_reopen_quotes){
    auto c=runner_config();c.max_inputs=1;c.cleanup_core_calls=4*c.core.capacity.orders+c.peer.max_trades+8;
    LocalQuoteRunner run(c);CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.step(StopLocalQuotes{},2),Error::None);CHECK(run.stopped());CHECK(run.trace().back().cleanup_lane);
    const auto before=run.core().canonical_state();
    CHECK_EQ(run.step(LocalRetryEvidenceInput{},3),Error::Capacity);CHECK(run.cleanup_exhausted());
    CHECK_EQ(run.core().canonical_state(),before);CHECK_EQ(run.step(pulse(c,2,4),4),Error::Halted);
}
TEST_CASE(RUNNER_cleanup_raw_input_replay_selects_same_lane_without_replaying_rejected_market){
    auto c=runner_config();c.max_inputs=1;LocalQuoteRunner run(c),replay(c);CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.step(pulse(c,2,2),2),Error::Capacity);
    CHECK_EQ(run.step(StopLocalQuotes{},3),Error::None);CHECK_EQ(run.step(LocalTradeQueryInput{{7,1},false},4),Error::None);
    for(const auto& f:run.trace()){
        CHECK_EQ(replay.step(f.input,f.at),f.error);CHECK_EQ(replay.trace().back().cleanup_lane,f.cleanup_lane);
        CHECK_EQ(replay.trace().back().canonical,f.canonical);
    }
    CHECK(replay.stopped());CHECK_EQ(replay.step(pulse(c,3,5),5),Error::Halted);
}
TEST_CASE(RUNNER_cleanup_rejects_time_reversal_without_consuming_reserved_record){
    auto c=runner_config();c.max_inputs=1;c.cleanup_inputs=1;LocalQuoteRunner run(c);CHECK_EQ(run.step(pulse(c),5),Error::None);
    CHECK_EQ(run.step(pulse(c,2,6),6),Error::Capacity);
    CHECK_EQ(run.step(StopLocalQuotes{},4),Error::OutOfOrderTime);CHECK_EQ(run.trace().size(),1U);
    CHECK_EQ(run.step(StopLocalQuotes{},7),Error::None);CHECK_EQ(run.trace().size(),2U);
    CHECK_EQ(run.step(StopLocalQuotes{},8),Error::Capacity);
}
TEST_CASE(RUNNER_cleanup_capacity_configuration_must_cover_a_whole_bounded_input){
    auto c=runner_config();c.cleanup_inputs=0;CHECK_THROWS(LocalQuoteRunner(c),std::invalid_argument);
    c=runner_config();c.cleanup_core_calls=1;CHECK_THROWS(LocalQuoteRunner(c),std::invalid_argument);
}
TEST_CASE(RUNNER_same_prices_after_fill_replace_open_sell_with_close_then_reduce_position){
    const auto c=runner_config();LocalQuoteRunner run(c);CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.step(LocalFillInput{{7,1},"route-after-fill",1,499},2),Error::None);
    CHECK_EQ(run.step(pulse(c,2,3),3),Error::None);
    CHECK_EQ(run.core().state().orders.at({7,2}).parts().released,1);
    CHECK_EQ(run.core().state().orders.at({7,4}).intent().offset,Offset::CloseToday);
    CHECK_EQ(run.core().state().positions.frozen()[0],1);
    CHECK_EQ(run.step(LocalFillInput{{7,4},"route-close",1,501},4),Error::None);
    CHECK_EQ(run.core().state().positions.booked()[0],0);CHECK_EQ(run.trace().back().peer->positions[0],0);
}
TEST_CASE(RUNNER_explicit_timer_withdraws_stale_quotes_without_new_market_input){
    const auto c=runner_config();LocalQuoteRunner run(c);CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.step(LocalQuoteTimer{},11),Error::None);CHECK(run.trace().back().calls.empty());
    CHECK_EQ(run.step(LocalQuoteTimer{},12),Error::StaleMark);CHECK_EQ(run.trace().back().calls.size(),6U);
    for(const auto& order:run.trace().back().orders)CHECK_EQ(order.parts.held(),0);
    CHECK_EQ(run.step(pulse(c,2,13),13),Error::None);CHECK_EQ(run.core().state().orders.size(),4U);
}
TEST_CASE(RUNNER_timer_without_market_and_repeated_expiry_cannot_create_orders){
    const auto c=runner_config();LocalQuoteRunner run(c);CHECK_EQ(run.step(LocalQuoteTimer{},0),Error::None);
    CHECK(run.trace().back().calls.empty());CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.step(LocalQuoteTimer{},12),Error::StaleMark);
    CHECK_EQ(run.step(LocalQuoteTimer{},13),Error::StaleMark);CHECK(run.trace().back().calls.empty());
    CHECK_EQ(run.core().state().orders.size(),2U);
}
TEST_CASE(RUNNER_deferred_fill_blocks_new_quotes_and_incomplete_query_cannot_book_or_clear_it){
    const auto c=runner_config();LocalQuoteRunner run(c);CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.step(LocalDeferredFillInput{{{7,1},"deferred-runner",1,499}},2),Error::None);
    CHECK_EQ(run.trace().back().pending_reports,1U);CHECK(run.trace().back().calls.empty());
    CHECK_EQ(run.core().state().positions.booked()[0],0);CHECK_EQ(run.trace().back().peer->positions[0],1);
    CHECK_EQ(run.step(pulse(c,2,3),3),Error::Blocked);CHECK_EQ(run.core().state().orders.size(),2U);
    for(const auto& call:run.trace().back().calls)CHECK(!std::holds_alternative<SubmitOrder>(call.input));
    const auto before=run.core().canonical_state();CHECK_EQ(run.step(LocalTradeQueryInput{{7,100},true},4),Error::Blocked);
    CHECK(run.trace().back().calls.empty());CHECK_EQ(run.trace().back().pending_reports,1U);CHECK_EQ(run.core().canonical_state(),before);
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,101},false},5),Error::None);
    CHECK_EQ(run.trace().back().pending_reports,0U);CHECK_EQ(run.core().state().positions.booked()[0],1);
    CHECK_EQ(run.step(LocalHeldReportInput{0},6),Error::Duplicate);CHECK_EQ(run.core().state().positions.booked()[0],1);
    CHECK_EQ(run.step(pulse(c,3,7),7),Error::None);CHECK_EQ(run.core().state().orders.size(),4U);
}
TEST_CASE(RUNNER_held_delivery_can_fill_gap_and_repeated_defer_does_not_add_pending_exposure){
    const auto c=runner_config();LocalQuoteRunner run(c);CHECK_EQ(run.step(pulse(c),1),Error::None);
    LocalDeferredFillInput fill{{{7,1},"held-repeat",1,499}};
    CHECK_EQ(run.step(fill,2),Error::None);CHECK_EQ(run.step(fill,3),Error::Duplicate);
    CHECK_EQ(run.trace().back().pending_reports,1U);CHECK_EQ(run.trace().back().peer->positions[0],1);
    CHECK_EQ(run.step(LocalHeldReportInput{0},4),Error::None);CHECK_EQ(run.trace().back().pending_reports,0U);
    CHECK_EQ(run.step(LocalHeldReportInput{0},5),Error::Duplicate);CHECK_EQ(run.core().state().positions.booked()[0],1);
}
TEST_CASE(RUNNER_invalid_query_and_delivery_do_not_create_core_events_or_resume_pending_quotes){
    const auto c=runner_config();LocalQuoteRunner run(c);CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.step(LocalDeferredFillInput{{{7,1},"query-invalid",1,499}},2),Error::None);
    const auto before=run.core().canonical_state();CHECK_EQ(run.step(LocalTradeQueryInput{{},false},3),Error::InvalidInput);
    CHECK(run.trace().back().calls.empty());CHECK_EQ(run.core().canonical_state(),before);
    CHECK_EQ(run.step(LocalHeldReportInput{9},4),Error::InvalidInput);CHECK_EQ(run.trace().back().pending_reports,1U);
}
TEST_CASE(RUNNER_deferred_query_path_raw_input_replay_preserves_divergence_and_gap_count){
    const auto c=runner_config();LocalQuoteRunner first(c),replay(c);
    CHECK_EQ(first.step(pulse(c),1),Error::None);
    CHECK_EQ(first.step(LocalDeferredFillInput{{{7,1},"query-replay",1,499}},2),Error::None);
    CHECK_EQ(first.step(LocalTradeQueryInput{{7,1},true},3),Error::Blocked);
    CHECK_EQ(first.step(LocalTradeQueryInput{{7,2},false},4),Error::None);
    CHECK_EQ(first.step(LocalHeldReportInput{0},5),Error::Duplicate);
    for(const auto& f:first.trace()){
        CHECK_EQ(replay.step(f.input,f.at),f.error);const auto& r=replay.trace().back();
        CHECK_EQ(r.canonical,f.canonical);CHECK_EQ(r.positions,f.positions);CHECK_EQ(r.peer->positions,f.peer->positions);
        CHECK_EQ(r.pending_reports,f.pending_reports);CHECK_EQ(r.calls.size(),f.calls.size());
    }
}
TEST_CASE(RUNNER_stop_attempts_both_orders_despite_first_terminal_commit_failure){
    const auto c=runner_config();LocalQuoteRunner run(c);CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.step(LocalReportFailureInput{1},2),Error::None);
    CHECK_EQ(run.step(StopLocalQuotes{},3),Error::InjectedFailure);
    CHECK_EQ(run.trace().back().pending_delivery,3U);
    CHECK_EQ(run.core().state().orders.at({7,1}).parts().possible,1);
    CHECK_EQ(run.trace().back().peer->orders[0].status,SimOrderStatus::Canceled);
    CHECK_EQ(run.trace().back().peer->orders[1].status,SimOrderStatus::Canceled);
    CHECK_EQ(run.core().state().orders.at({7,2}).cancel(),CancelState::Requested);
    CHECK_EQ(run.step(LocalRetryEvidenceInput{},4),Error::None);CHECK_EQ(run.trace().back().pending_delivery,0U);
    CHECK_EQ(run.core().state().orders.at({7,1}).parts().released,1);
    CHECK_EQ(run.core().state().orders.at({7,2}).parts().released,1);
    CHECK_EQ(run.step(StopLocalQuotes{},5),Error::None);CHECK_EQ(run.core().state().orders.at({7,2}).parts().released,1);
}
TEST_CASE(RUNNER_query_batch_failure_retains_unattempted_trade_and_blocks_new_orders){
    const auto c=runner_config();LocalQuoteRunner run(c);CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.step(LocalDeferredFillInput{{{7,1},"inbox-buy",1,499}},2),Error::None);
    CHECK_EQ(run.step(LocalDeferredFillInput{{{7,2},"inbox-sell",1,501}},3),Error::None);
    CHECK_EQ(run.step(LocalReportFailureInput{0},4),Error::None);
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,100},false},5),Error::InjectedFailure);
    CHECK_EQ(run.trace().back().pending_delivery,2U);CHECK_EQ(run.trace().back().pending_reports,2U);
    CHECK_EQ(run.core().state().executions.records().size(),0U);
    CHECK_EQ(run.step(pulse(c,2,6),6),Error::Blocked);CHECK(run.trace().back().calls.empty());
    CHECK_EQ(run.step(LocalRetryEvidenceInput{},7),Error::None);CHECK_EQ(run.trace().back().pending_delivery,0U);
    CHECK_EQ(run.trace().back().pending_reports,0U);CHECK_EQ(run.core().state().executions.records().size(),2U);
    CHECK(run.trace().back().query_blocked);CHECK_EQ(run.step(LocalTradeQueryInput{{7,101},false},8),Error::None);
    CHECK(!run.trace().back().query_blocked);
}
TEST_CASE(RUNNER_inbox_overflow_cannot_be_cleared_by_draining_or_a_later_complete_query){
    auto c=runner_config();c.core=config({10,0,0,0});c.core.trade_evidence_mode=TradeEvidenceMode::Simulator;
    c.core.capacity.orders=1;c.peer=example::simulator_config(c.core);c.peer.max_trades=1;
    LocalQuoteRunner run(c);CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.step(LocalReportFailureInput{0},2),Error::None);
    CHECK_EQ(run.step(LocalFillInput{{7,1},"overflow-close",1,501},3),Error::InjectedFailure);
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,1},false,true},4),Error::Blocked);
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,2},false,true},5),Error::Blocked);
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,3},false,true},6),Error::Capacity);CHECK(run.trace().back().delivery_loss);
    CHECK_EQ(run.step(LocalRetryEvidenceInput{},7),Error::None);CHECK_EQ(run.trace().back().pending_delivery,0U);
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,4},false},8),Error::Blocked);
    CHECK(run.trace().back().delivery_loss);CHECK(run.trace().back().query_blocked);
    CHECK_EQ(run.step(pulse(c,2,9),9),Error::Blocked);CHECK(run.trace().back().calls.empty());
}
TEST_CASE(RUNNER_evidence_inbox_batch_capacity_rejection_keeps_existing_receipts_intact){
    LocalEvidenceInbox inbox(2);CHECK_EQ(inbox.enqueue({ReferenceMark{500}}),Error::None);
    CHECK_EQ(inbox.enqueue({KillRequested{},ReferenceMark{501}}),Error::Capacity);CHECK_EQ(inbox.size(),1U);
    CHECK_EQ(std::get<ReferenceMark>(inbox.front()).price,500);inbox.acknowledge();CHECK_EQ(inbox.size(),0U);
}
TEST_CASE(RUNNER_market_withdraw_failure_still_cancels_both_peer_orders_before_retry){
    const auto c=runner_config();LocalQuoteRunner run(c);CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.step(LocalReportFailureInput{0},2),Error::None);
    auto unhealthy=pulse(c,2,3);unhealthy.book.healthy=false;
    CHECK_EQ(run.step(unhealthy,3),Error::InjectedFailure);CHECK_EQ(run.trace().back().pending_delivery,4U);
    CHECK_EQ(run.trace().back().peer->orders[0].status,SimOrderStatus::Canceled);
    CHECK_EQ(run.trace().back().peer->orders[1].status,SimOrderStatus::Canceled);
    CHECK_EQ(run.step(pulse(c,3,4),4),Error::Blocked);CHECK(run.trace().back().calls.empty());
    CHECK_EQ(run.step(LocalRetryEvidenceInput{},5),Error::None);
    for(const auto& o:run.trace().back().orders)CHECK_EQ(o.parts.held(),0);
    CHECK_EQ(run.step(pulse(c,3,6),6),Error::None);CHECK_EQ(run.core().state().orders.size(),4U);
}
TEST_CASE(RUNNER_stop_with_peer_cancel_capacity_error_retains_unconfirmed_order_risk){
    auto c=runner_config();c.peer.max_cancels=1;LocalQuoteRunner run(c);CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.step(StopLocalQuotes{},2),Error::Capacity);CHECK_EQ(run.core().state().mode,CoreMode::Killed);
    CHECK_EQ(run.core().state().orders.at({7,1}).parts().released,1);
    CHECK_EQ(run.core().state().orders.at({7,2}).parts().possible,1);
    CHECK_EQ(run.trace().back().peer->orders[1].status,SimOrderStatus::Working);
    CHECK_EQ(run.step(LocalRetryEvidenceInput{},3),Error::None);
    CHECK_EQ(run.core().state().orders.at({7,2}).parts().possible,1);
    const auto before=run.core().canonical_state();
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,1},false},4),Error::Blocked);
    CHECK_EQ(run.trace().back().query_aligned,std::optional<bool>{false});CHECK(run.trace().back().query_blocked);
    CHECK(run.trace().back().calls.empty());CHECK_EQ(run.core().canonical_state(),before);
}
TEST_CASE(RUNNER_pending_terminal_cannot_clear_query_stop_until_receipt_retry_and_fresh_review){
    const auto c=runner_config();LocalQuoteRunner run(c);CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.step(LocalReportFailureInput{1},2),Error::None);
    CHECK_EQ(run.step(StopLocalQuotes{},3),Error::InjectedFailure);
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,1},false},4),Error::Blocked);
    CHECK(!run.trace().back().query_aligned.has_value());CHECK(run.trace().back().query_blocked);
    CHECK_EQ(run.step(LocalRetryEvidenceInput{},5),Error::None);CHECK(run.trace().back().query_blocked);
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,2},false},6),Error::None);
    CHECK_EQ(run.trace().back().query_aligned,std::optional<bool>{true});CHECK(!run.trace().back().query_blocked);
    CHECK_EQ(run.core().state().mode,CoreMode::Killed);
}

TEST_CASE(RUNNER_repeated_normal_queries_do_not_duplicate_failed_receipts_or_invent_delivery_loss){
    const auto c=example::quote_overflow_config();LocalQuoteRunner run(c);
    CHECK_EQ(run.step(example::runner_pulse(c,1,1,499,501),1),Error::None);
    CHECK_EQ(run.step(LocalReportFailureInput{0},2),Error::None);
    CHECK_EQ(run.step(LocalFillInput{{7,1},"query-backpressure",1,501},3),Error::InjectedFailure);
    const auto before=run.core().canonical_state();
    for(std::uint64_t i=0;i<5;++i){
        CHECK_EQ(run.step(LocalTradeQueryInput{{7,i+1},false},4+i),Error::Blocked);
        CHECK_EQ(run.trace().back().pending_delivery,1U);CHECK(!run.trace().back().delivery_loss);
        CHECK(run.trace().back().calls.empty());CHECK_EQ(run.core().canonical_state(),before);
    }
    CHECK_EQ(run.step(LocalRetryEvidenceInput{},9),Error::None);
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,100},false},10),Error::None);
    CHECK_EQ(run.trace().back().query_aligned,std::optional<bool>{true});CHECK(!run.trace().back().query_blocked);
}
TEST_CASE(RUNNER_query_backpressure_is_preserved_in_cleanup_reserve){
    auto c=runner_config();c.max_inputs=3;LocalQuoteRunner run(c);
    CHECK_EQ(run.step(pulse(c),1),Error::None);CHECK_EQ(run.step(LocalReportFailureInput{0},2),Error::None);
    CHECK_EQ(run.step(LocalFillInput{{7,1},"cleanup-backpressure",1,499},3),Error::InjectedFailure);
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,1},false},4),Error::Blocked);CHECK(run.trace().back().cleanup_lane);
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,2},false},5),Error::Blocked);CHECK_EQ(run.trace().back().pending_delivery,1U);
    CHECK_EQ(run.step(LocalRetryEvidenceInput{},6),Error::None);
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,3},false},7),Error::None);CHECK(!run.trace().back().query_blocked);
    CHECK(run.stopped());CHECK_EQ(run.step(pulse(c,2,8),8),Error::Halted);
}
TEST_CASE(RUNNER_backpressured_query_inputs_replay_without_queue_growth){
    auto c=runner_config();LocalQuoteRunner first(c),replay(c);
    CHECK_EQ(first.step(pulse(c),1),Error::None);CHECK_EQ(first.step(LocalReportFailureInput{0},2),Error::None);
    CHECK_EQ(first.step(LocalFillInput{{7,1},"backpressure-replay",1,499},3),Error::InjectedFailure);
    CHECK_EQ(first.step(LocalTradeQueryInput{{7,1},false},4),Error::Blocked);
    CHECK_EQ(first.step(LocalTradeQueryInput{{7,2},false},5),Error::Blocked);
    CHECK_EQ(first.step(LocalRetryEvidenceInput{},6),Error::None);
    for(const auto& f:first.trace()){
        CHECK_EQ(replay.step(f.input,f.at),f.error);CHECK_EQ(replay.trace().back().pending_delivery,f.pending_delivery);
        CHECK_EQ(replay.trace().back().canonical,f.canonical);CHECK_EQ(replay.trace().back().delivery_loss,f.delivery_loss);
    }
}
TEST_CASE(RUNNER_held_delivery_commit_failure_is_retained_for_explicit_retry){
    const auto c=runner_config();LocalQuoteRunner run(c);
    CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.step(LocalDeferredFillInput{{{7,1},"held-retry",1,499}},2),Error::None);
    CHECK_EQ(run.step(LocalReportFailureInput{0},3),Error::None);
    CHECK_EQ(run.step(LocalHeldReportInput{0},4),Error::InjectedFailure);
    CHECK_EQ(run.trace().back().pending_delivery,1U);
    CHECK_EQ(run.trace().back().pending_reports,1U);
    CHECK_EQ(run.core().state().positions.booked()[0],0);
    CHECK_EQ(run.step(LocalRetryEvidenceInput{},5),Error::None);
    CHECK_EQ(run.trace().back().pending_delivery,0U);
    CHECK_EQ(run.trace().back().pending_reports,0U);
    CHECK_EQ(run.core().state().positions.booked()[0],1);
    CHECK_EQ(run.step(LocalHeldReportInput{0},6),Error::Duplicate);
    CHECK_EQ(run.core().state().positions.booked()[0],1);
}
TEST_CASE(RUNNER_held_delivery_waits_behind_failed_receipts_and_replays_in_order){
    const auto c=runner_config();LocalQuoteRunner first(c),replay(c);
    CHECK_EQ(first.step(pulse(c),1),Error::None);
    CHECK_EQ(first.step(LocalDeferredFillInput{{{7,1},"held-queued",1,499}},2),Error::None);
    CHECK_EQ(first.step(LocalReportFailureInput{0},3),Error::None);
    CHECK_EQ(first.step(LocalFillInput{{7,2},"push-first",1,501},4),Error::InjectedFailure);
    const auto before=first.core().canonical_state();
    CHECK_EQ(first.step(LocalHeldReportInput{0},5),Error::Blocked);
    CHECK(first.trace().back().calls.empty());
    CHECK_EQ(first.trace().back().pending_delivery,2U);
    CHECK_EQ(first.core().canonical_state(),before);
    CHECK_EQ(first.step(LocalRetryEvidenceInput{},6),Error::None);
    CHECK_EQ(first.core().state().positions.booked()[0],1);
    CHECK_EQ(first.core().state().positions.booked()[2],1);
    for(const auto& f:first.trace()){
        CHECK_EQ(replay.step(f.input,f.at),f.error);
        CHECK_EQ(replay.trace().back().canonical,f.canonical);
        CHECK_EQ(replay.trace().back().pending_delivery,f.pending_delivery);
    }
}
