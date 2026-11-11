#include "fixture.hpp"
#include "../examples/local_sim_scenario.hpp"
#include "p03/local_quote_runner.hpp"
#include "p03/local_quote_replay.hpp"
#include "../examples/quote_runner_scenario.hpp"
using namespace fixture;
TEST_CASE(RUNNER_missing_cancel_terminal_is_recovered_from_valid_full_query_without_cancel_action_fabrication){
    const auto c=example::quote_runner_config();LocalQuoteRunner run(c);
    CHECK_EQ(run.step(example::runner_pulse(c,1,1,499,501),1),Error::None);
    CHECK_EQ(run.step(LocalCancelTerminalGapInput{},2),Error::None);
    CHECK_EQ(run.step(StopLocalQuotes{},3),Error::Blocked);
    CHECK_EQ(run.core().state().mode,CoreMode::Killed);CHECK(run.trace().back().query_blocked);
    CHECK_EQ(run.core().state().orders.at({7,1}).parts().possible,1);
    CHECK_EQ(run.core().state().orders.at({7,1}).cancel(),CancelState::Succeeded);
    CHECK_EQ(run.trace().back().peer->orders[0].status,SimOrderStatus::Canceled);
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,1},false},4),Error::None);
    CHECK_EQ(run.trace().back().query_aligned,std::optional<bool>{true});CHECK(!run.trace().back().query_blocked);
    CHECK_EQ(run.core().state().orders.at({7,1}).parts().released,1);CHECK_EQ(run.trace().back().calls.size(),1U);
    const auto& terminal=std::get<TerminalReport>(run.trace().back().calls[0].input);
    CHECK_EQ(terminal.header.stamp.source,std::string("localsim.query.orders"));CHECK_EQ(terminal.proof.exact_quantity,std::optional<Lots>{0});
    CHECK_EQ(run.core().state().mode,CoreMode::Killed);CHECK_EQ(verify_local_quote_trace(c,run.trace()).error,Error::None);
}
TEST_CASE(RUNNER_missing_cancel_terminal_and_trade_require_complete_query_before_release){
    auto c=example::quote_runner_config();c.strategy.quantity=3;LocalQuoteRunner run(c);
    CHECK_EQ(run.step(example::runner_pulse(c,1,1,499,501),1),Error::None);
    CHECK_EQ(run.step(LocalDeferredFillInput{{{7,1},"cancel-query-fill",1,499}},2),Error::None);
    CHECK_EQ(run.step(LocalCancelTerminalGapInput{},3),Error::None);CHECK_EQ(run.step(StopLocalQuotes{},4),Error::Blocked);
    CHECK_EQ(run.core().state().orders.at({7,1}).parts(),(QuantityParts{0,0,3,0,0}));
    const auto before=run.core().canonical_state();CHECK_EQ(run.step(LocalTradeQueryInput{{7,1},true},5),Error::Blocked);
    CHECK(run.trace().back().calls.empty());CHECK_EQ(run.core().canonical_state(),before);
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,2},false},6),Error::None);CHECK_EQ(run.trace().back().calls.size(),2U);
    CHECK(std::holds_alternative<TradeReport>(run.trace().back().calls[0].input));CHECK(std::holds_alternative<TerminalReport>(run.trace().back().calls[1].input));
    CHECK_EQ(run.core().state().orders.at({7,1}).parts(),(QuantityParts{1,0,0,2,0}));
    CHECK_EQ(run.step(LocalHeldReportInput{0},7),Error::Duplicate);CHECK_EQ(run.core().state().positions.booked()[0],1);
    CHECK_EQ(run.core().state().mode,CoreMode::Killed);CHECK_EQ(verify_local_quote_trace(c,run.trace()).error,Error::None);
}
TEST_CASE(RUNNER_query_terminal_commit_failure_retains_receipt_and_requires_retry_then_review){
    auto c=example::quote_runner_config();c.strategy.quantity=3;LocalQuoteRunner run(c);
    CHECK_EQ(run.step(example::runner_pulse(c,1,1,499,501),1),Error::None);
    CHECK_EQ(run.step(LocalDeferredFillInput{{{7,1},"terminal-retry-fill",1,499}},2),Error::None);
    CHECK_EQ(run.step(LocalCancelTerminalGapInput{},3),Error::None);CHECK_EQ(run.step(StopLocalQuotes{},4),Error::Blocked);
    CHECK_EQ(run.step(LocalReportFailureInput{1},5),Error::None);
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,1},false},6),Error::InjectedFailure);
    CHECK_EQ(run.trace().back().pending_delivery,1U);CHECK_EQ(run.core().state().positions.booked()[0],1);
    CHECK_EQ(run.core().state().orders.at({7,1}).parts().possible,2);CHECK(run.trace().back().query_blocked);
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,2},false},7),Error::Blocked);CHECK(run.trace().back().calls.empty());
    CHECK_EQ(run.step(LocalRetryEvidenceInput{},8),Error::None);CHECK_EQ(run.trace().back().pending_delivery,0U);CHECK(run.trace().back().query_blocked);
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,3},false},9),Error::None);CHECK(!run.trace().back().query_blocked);
    CHECK_EQ(run.core().state().mode,CoreMode::Killed);CHECK_EQ(verify_local_quote_trace(c,run.trace()).error,Error::None);
}
TEST_CASE(EXPERIMENT_cleanup_replays_all_nine_attempts_including_three_rejections){
    const auto c=example::quote_cleanup_config();LocalQuoteRunner run(c);std::vector<LocalQuoteAttempt> attempts;
    CHECK(example::run_quote_cleanup_scenario(run,c,nullptr,&attempts));CHECK_EQ(attempts.size(),9U);
    const auto review=verify_local_quote_experiment(c,attempts,run.trace());CHECK_EQ(review.error,Error::None);
    CHECK_EQ(review.matched_attempts,9U);CHECK_EQ(review.matched_frames,6U);CHECK(attempts.back().cleanup_exhausted);
    CHECK_EQ(attempts.at(3).frames_before,attempts.at(3).frames_after);CHECK(attempts.at(3).stopped);
    CHECK_EQ(verify_local_quote_trace(c,run.trace()).error,Error::None);
}
TEST_CASE(EXPERIMENT_rejected_attempt_result_and_capacity_state_mismatches_are_located){
    const auto c=example::quote_cleanup_config();LocalQuoteRunner run(c);std::vector<LocalQuoteAttempt> attempts;
    CHECK(example::run_quote_cleanup_scenario(run,c,nullptr,&attempts));
    const auto reject=[&](std::size_t index,LocalReplayMismatch mismatch,auto mutate){auto a=attempts;mutate(a[index]);
        const auto r=verify_local_quote_experiment(c,a,run.trace());CHECK(r.error!=Error::None);CHECK_EQ(r.attempt,index+1);CHECK_EQ(r.matched_attempts,index);CHECK_EQ(r.mismatch,mismatch);};
    reject(3,LocalReplayMismatch::AttemptResult,[](auto& a){a.error=Error::None;});
    reject(3,LocalReplayMismatch::RunnerState,[](auto& a){a.stopped=false;});
    reject(8,LocalReplayMismatch::RunnerState,[](auto& a){a.cleanup_exhausted=false;});
    reject(7,LocalReplayMismatch::CoreState,[](auto& a){a.canonical+="changed";});
    reject(4,LocalReplayMismatch::AttemptShape,[](auto& a){a.frames_before=0;});
    reject(3,LocalReplayMismatch::AttemptShape,[](auto& a){a.ordinal=8;});
    reject(5,LocalReplayMismatch::AttemptResult,[](auto& a){a.at=0;});
    auto frames=run.trace();std::get<MarketPulse>(frames[0].input).book.bid_quantity+=1;
    CHECK_EQ(verify_local_quote_experiment(c,attempts,frames).mismatch,LocalReplayMismatch::RecordedInput);
}
TEST_CASE(EXPERIMENT_empty_attempts_and_bounded_capacity_are_explicit){
    const auto c=example::quote_cleanup_config();LocalQuoteRunner run(c);std::vector<LocalQuoteAttempt> attempts;
    CHECK(example::run_quote_cleanup_scenario(run,c,nullptr,&attempts));
    CHECK_EQ(verify_local_quote_experiment(c,{},run.trace()).mismatch,LocalReplayMismatch::EmptyTrace);
    const std::vector<LocalQuoteAttempt> oversized(4097);
    CHECK_EQ(verify_local_quote_experiment(c,oversized,run.trace()).mismatch,LocalReplayMismatch::TraceCapacity);
    auto bad=c;bad.max_inputs=0;CHECK_EQ(verify_local_quote_experiment(bad,attempts,run.trace()).mismatch,LocalReplayMismatch::InvalidConfiguration);
}
TEST_CASE(REPLAY_all_eight_scenarios_verify_recorded_frames_with_fresh_runner){
    const auto check=[](const LocalQuoteRunnerConfig& c,auto scenario){
        LocalQuoteRunner run(c);CHECK(scenario(run,c));const auto before=run.core().canonical_state();
        const auto review=verify_local_quote_trace(c,run.trace());CHECK_EQ(review.error,Error::None);
        CHECK_EQ(review.mismatch,LocalReplayMismatch::None);CHECK_EQ(review.matched_frames,run.trace().size());CHECK_EQ(run.core().canonical_state(),before);
    };
    const auto c=example::quote_runner_config();
    check(c,example::run_quote_scenario);check(c,example::run_quote_expiry_scenario);check(c,example::run_quote_query_scenario);check(c,example::run_quote_retry_scenario);
    check(example::quote_overflow_config(),example::run_quote_overflow_scenario);
    check(example::quote_cleanup_config(),[](LocalQuoteRunner& run,const LocalQuoteRunnerConfig& config){return example::run_quote_cleanup_scenario(run,config);});
    check(example::quote_late_fill_config(),example::run_quote_late_fill_scenario);check(example::quote_inventory_config(),example::run_quote_inventory_scenario);
}
TEST_CASE(REPLAY_reports_first_frame_and_category_for_independent_observation_corruption){
    const auto c=example::quote_inventory_config();LocalQuoteRunner run(c);CHECK(example::run_quote_inventory_scenario(run,c));
    const auto reject=[&](std::size_t index,LocalReplayMismatch mismatch,auto mutate){
        auto trace=run.trace();mutate(trace.at(index));const auto r=verify_local_quote_trace(c,trace);
        CHECK(r.error!=Error::None);CHECK_EQ(r.frame,index+1);CHECK_EQ(r.matched_frames,index);CHECK_EQ(r.mismatch,mismatch);
    };
    reject(0,LocalReplayMismatch::FrameShape,[](auto& f){f.ordinal=0;});
    reject(0,LocalReplayMismatch::FrameShape,[](auto& f){f.completed=false;});
    reject(1,LocalReplayMismatch::StepError,[](auto& f){f.error=Error::Blocked;});
    reject(0,LocalReplayMismatch::CoreState,[](auto& f){f.canonical+="changed";});
    reject(0,LocalReplayMismatch::CoreState,[](auto& f){f.core_required+=1;});
    reject(0,LocalReplayMismatch::StrategyDecision,[](auto& f){f.decision->target->buy->quantity+=1;});
    reject(0,LocalReplayMismatch::CoreCalls,[](auto& f){std::get<SubmitOrder>(f.calls.at(1).input).intent.price+=1;});
    reject(0,LocalReplayMismatch::CoreCalls,[](auto& f){f.calls.at(1).result.command.serial+=1;});
    reject(0,LocalReplayMismatch::OrderViews,[](auto& f){f.orders.at(0).parts.possible+=1;});
    reject(0,LocalReplayMismatch::PeerSnapshot,[](auto& f){f.peer->orders.at(0).reserved_fees+=1;});
    reject(1,LocalReplayMismatch::PeerSnapshot,[](auto& f){f.peer->trades.at(0).channel=Channel::Push;});
    reject(0,LocalReplayMismatch::DeliveryState,[](auto& f){f.pending_delivery+=1;});
    reject(5,LocalReplayMismatch::DeliveryState,[](auto& f){f.query_aligned=false;});
}
TEST_CASE(REPLAY_empty_oversized_invalid_configuration_and_valid_prefix_are_distinguished){
    const auto c=example::quote_inventory_config();LocalQuoteRunner run(c);CHECK(example::run_quote_inventory_scenario(run,c));
    CHECK_EQ(verify_local_quote_trace(c,{}).mismatch,LocalReplayMismatch::EmptyTrace);
    auto bad=c;bad.max_inputs=0;CHECK_EQ(verify_local_quote_trace(bad,run.trace()).mismatch,LocalReplayMismatch::InvalidConfiguration);
    bad=c;bad.max_inputs=1;bad.cleanup_inputs=1;CHECK_EQ(verify_local_quote_trace(bad,run.trace()).mismatch,LocalReplayMismatch::TraceCapacity);
    const auto prefix=std::span<const LocalQuoteFrame>{run.trace()}.first(3);const auto r=verify_local_quote_trace(c,prefix);
    CHECK_EQ(r.error,Error::None);CHECK_EQ(r.matched_frames,3U);
}
TEST_CASE(RUNNER_inventory_export_scenario_replays_seven_frames){
    const auto c=example::quote_inventory_config();LocalQuoteRunner run(c),replay(c);
    CHECK(example::run_quote_inventory_scenario(run,c));CHECK_EQ(run.trace().size(),7U);
    for(const auto& f:run.trace()){
        CHECK_EQ(replay.step(f.input,f.at),f.error);CHECK_EQ(replay.trace().back().canonical,f.canonical);
        CHECK_EQ(replay.trace().back().positions,f.positions);CHECK_EQ(replay.trace().back().calls.size(),f.calls.size());
    }
}
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
TEST_CASE(RUNNER_repeated_held_delivery_does_not_multiply_failed_receipts){
    const auto c=runner_config();LocalQuoteRunner run(c);
    CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.step(LocalDeferredFillInput{{{7,1},"held-repeat",1,499}},2),Error::None);
    CHECK_EQ(run.step(LocalReportFailureInput{0},3),Error::None);
    CHECK_EQ(run.step(LocalHeldReportInput{0},4),Error::InjectedFailure);
    const auto before=run.core().canonical_state();
    for(std::uint64_t at=5;at<15;++at){
        CHECK_EQ(run.step(LocalHeldReportInput{0},at),Error::Blocked);
        CHECK_EQ(run.trace().back().pending_delivery,1U);
        CHECK(!run.trace().back().delivery_loss);CHECK(run.trace().back().calls.empty());
        CHECK_EQ(run.core().canonical_state(),before);
    }
    CHECK_EQ(run.step(LocalRetryEvidenceInput{},15),Error::None);
    CHECK_EQ(run.core().state().positions.booked()[0],1);
    CHECK_EQ(run.step(LocalHeldReportInput{0},16),Error::Duplicate);
}
TEST_CASE(RUNNER_inbox_exact_match_preserves_changed_payload_and_evidence){
    const auto c=runner_config();LocalQuoteRunner run(c);
    CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.step(LocalDeferredFillInput{{{7,1},"exact-match",1,499}},2),Error::None);
    auto report=run.trace().back().peer->trades.at(0);LocalEvidenceInbox inbox(4);
    CHECK_EQ(inbox.enqueue({BusinessEvent{report}}),Error::None);CHECK(inbox.contains_exact(report));
    auto changed=report;changed.trade.price+=1;CHECK(!inbox.contains_exact(changed));
    changed=report;changed.stamp.source+=".changed";CHECK(!inbox.contains_exact(changed));
    changed=report;changed.stamp.epoch+=1;CHECK(!inbox.contains_exact(changed));
    changed=report;changed.channel=report.channel==Channel::Push?Channel::Query:Channel::Push;
    CHECK(inbox.contains_exact(report));CHECK(!inbox.contains_exact(changed));
    CHECK_EQ(inbox.enqueue({BusinessEvent{changed}}),Error::None);CHECK_EQ(inbox.size(),2U);
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
TEST_CASE(RUNNER_inventory_boundary_partial_fills_requote_query_and_stop_replay_end_to_end){
    for(const bool short_side:{false,true}){
        auto c=runner_config();c.core=config(short_side?std::array<Lots,4>{0,0,9,0}:std::array<Lots,4>{9,0,0,0});
        c.core.trade_evidence_mode=TradeEvidenceMode::Simulator;c.peer=example::simulator_config(c.core);c.strategy.quantity=3;
        LocalQuoteRunner first(c),replay(c);CHECK_EQ(first.step(pulse(c),1),Error::None);
        CHECK_EQ(first.core().state().orders.at({7,1}).intent().quantity,short_side?3:1);
        CHECK_EQ(first.core().state().orders.at({7,2}).intent().quantity,short_side?1:3);
        const OrderId increasing{7,short_side?2U:1U},reducing{7,short_side?1U:2U};
        CHECK_EQ(first.step(LocalFillInput{increasing,"boundary-increase",1,short_side?501:499},2),Error::None);
        CHECK_EQ(first.step(pulse(c,2,3),3),Error::None);CHECK_EQ(first.core().state().orders.size(),2U);
        CHECK_EQ(first.step(LocalFillInput{reducing,"boundary-reduce",1,short_side?499:501},4),Error::None);
        CHECK_EQ(first.step(pulse(c,3,5),5),Error::None);CHECK_EQ(first.core().state().orders.size(),4U);
        bool submitted=false;bool canceled=false;
        for(const auto& call:first.trace().back().calls){
            if(std::holds_alternative<RequestCancel>(call.input)){CHECK(!submitted);canceled=true;}
            if(std::holds_alternative<SubmitOrder>(call.input))submitted=true;
        }
        CHECK(canceled);CHECK(submitted);
        CHECK_EQ(first.step(LocalTradeQueryInput{{7,100},false},6),Error::None);
        CHECK_EQ(first.trace().back().query_aligned,std::optional<bool>{true});
        CHECK_EQ(first.step(StopLocalQuotes{},7),Error::None);
        for(const auto& frame:first.trace()){
            CHECK_EQ(frame.positions,frame.peer->positions);CHECK(frame.completed);
            const auto net=frame.positions[0]+frame.positions[1]-frame.positions[2]-frame.positions[3];CHECK(net>=-10 && net<=10);
            CHECK_EQ(replay.step(frame.input,frame.at),frame.error);CHECK_EQ(replay.trace().back().canonical,frame.canonical);
        }
        CHECK_EQ(first.core().state().mode,CoreMode::Killed);
        for(const auto& o:first.trace().back().orders)CHECK_EQ(o.parts.held(),0);
    }
}
TEST_CASE(RUNNER_lot_clipped_partial_close_replacement_is_valid_on_core_and_peer){
    auto c=runner_config();c.core=config({4,3,0,0});c.core.trade_evidence_mode=TradeEvidenceMode::Simulator;
    c.core.contract.min_lots=2;c.core.contract.lot_step=2;c.peer=example::simulator_config(c.core);c.strategy.quantity=4;
    LocalQuoteRunner run(c);CHECK_EQ(run.step(pulse(c),1),Error::None);
    CHECK_EQ(run.core().state().orders.at({7,2}).intent().quantity,2);
    CHECK_EQ(run.step(LocalFillInput{{7,2},"lot-partial-close",1,501},2),Error::None);
    CHECK_EQ(run.step(pulse(c,2,3),3),Error::None);
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,1},false},4),Error::None);
    CHECK_EQ(run.trace().back().query_aligned,std::optional<bool>{true});
    for(const auto& [id,order]:run.core().state().orders){(void)id;CHECK_EQ(c.core.contract.validate(order.intent()),Error::None);}
    CHECK_EQ(run.step(StopLocalQuotes{},5),Error::None);CHECK_EQ(run.trace().back().positions,run.trace().back().peer->positions);
}

namespace {
LocalQuoteRunnerConfig qualified_config(){auto c=example::quote_runner_config();
    c.market=MarketStateConfig{c.strategy.scope,c.core.contract,c.strategy.origin,c.strategy.source,c.strategy.market_epoch,1,2,c.strategy.max_age};return c;}
LocalMarketSnapshotInput qualified_snapshot(const LocalQuoteRunnerConfig& c,std::uint64_t seq,LogicalTime at,std::uint64_t generation=1){
    MarketSnapshot b;b.scope=c.strategy.scope;b.origin=c.strategy.origin;b.source=c.strategy.source;b.source_epoch=c.strategy.market_epoch;
    b.subscription_generation=generation;b.receive_sequence=seq;b.received_at=at;b.valid_depth=2;b.valid_fields=market_required_fields;
    b.bids[0]={499,10};b.bids[1]={498,10};b.asks[0]={501,10};b.asks[1]={502,10};
    return {b,example::runner_pulse(c,seq,at,499,501).session};
}
}
TEST_CASE(A02_RUNNER_disconnect_withdraws_and_only_fresh_new_subscription_can_quote){
    const auto c=qualified_config();LocalQuoteRunner run(c);
    CHECK_EQ(run.step(qualified_snapshot(c,100,1),1),Error::None);CHECK_EQ(run.trace().back().feed->status,FeedStatus::Valid);
    CHECK_EQ(run.step(LocalMarketDisconnectInput{},2),Error::None);CHECK_EQ(run.trace().back().feed->status,FeedStatus::Recovering);
    for(const auto& [id,o]:run.core().state().orders){(void)id;CHECK_EQ(o.parts().possible,0);}
    CHECK_EQ(run.step(qualified_snapshot(c,101,3),3),Error::Blocked);
    CHECK_EQ(run.step(LocalMarketSubscriptionInput{2},4),Error::None);CHECK_EQ(run.trace().back().feed->status,FeedStatus::Recovering);
    CHECK_EQ(run.step(qualified_snapshot(c,102,5),5),Error::WrongIdentity);
    CHECK_EQ(run.step(qualified_snapshot(c,1,6,2),6),Error::None);CHECK_EQ(run.trace().back().feed->status,FeedStatus::Valid);
    CHECK_EQ(run.step(LocalTradeQueryInput{{7,1},false},7),Error::None);CHECK_EQ(run.trace().back().query_aligned,std::optional<bool>{true});
    CHECK_EQ(verify_local_quote_trace(c,run.trace()).error,Error::None);
    auto changed=run.trace();changed[1].feed->subscription_generation=99;
    CHECK_EQ(verify_local_quote_trace(c,changed).mismatch,LocalReplayMismatch::RunnerState);
}
TEST_CASE(A02_RUNNER_invalid_depth_and_expiry_withdraw_and_legacy_input_cannot_bypass){
    const auto c=qualified_config();LocalQuoteRunner run(c);
    CHECK_EQ(run.step(qualified_snapshot(c,1,1),1),Error::None);
    auto bad=qualified_snapshot(c,2,2);bad.snapshot.valid_depth=1;
    CHECK_EQ(run.step(bad,2),Error::InvalidInput);CHECK_EQ(run.trace().back().feed->status,FeedStatus::Invalid);
    CHECK_EQ(run.step(example::runner_pulse(c,3,3,499,501),3),Error::InvalidInput);CHECK(run.trace().back().calls.empty());
    CHECK_EQ(run.step(qualified_snapshot(c,3,4),4),Error::None);
    CHECK_EQ(run.step(LocalQuoteTimer{},15),Error::StaleMark);CHECK_EQ(run.trace().back().feed->status,FeedStatus::Stale);
    CHECK_EQ(verify_local_quote_trace(c,run.trace()).error,Error::None);
}
TEST_CASE(A02_RUNNER_explicit_loss_cannot_be_healed_by_same_generation){
    const auto c=qualified_config();LocalQuoteRunner run(c);CHECK_EQ(run.step(qualified_snapshot(c,1,1),1),Error::None);
    CHECK_EQ(run.step(LocalMarketLossInput{},2),Error::None);CHECK_EQ(run.trace().back().feed->reason,FeedReason::LocalLoss);
    CHECK_EQ(run.step(qualified_snapshot(c,2,3),3),Error::Blocked);CHECK(run.trace().back().calls.empty());
    CHECK_EQ(verify_local_quote_trace(c,run.trace()).error,Error::None);
    auto wrong=c;wrong.market->source="different";CHECK_THROWS(LocalQuoteRunner(wrong),std::invalid_argument);
}

TEST_CASE(A02_RUNNER_feed_demonstration_preserves_raw_input_and_replays_all_twelve_frames){
    const auto c=example::quote_feed_config();LocalQuoteRunner run(c);CHECK(example::run_quote_feed_scenario(run,c));
    CHECK_EQ(verify_local_quote_trace(c,run.trace()).error,Error::None);
    auto changed=run.trace();std::get<LocalMarketSnapshotInput>(changed[0].input).snapshot.valid_depth=1;
    CHECK_EQ(verify_local_quote_trace(c,changed).mismatch,LocalReplayMismatch::StepError);
}
