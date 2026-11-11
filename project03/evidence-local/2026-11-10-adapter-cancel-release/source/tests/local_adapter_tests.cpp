#include "fixture.hpp"
#include "p03/local_adapter_session.hpp"
#include "p03/local_query_review.hpp"
#include "../examples/quote_runner_scenario.hpp"
using namespace fixture;
namespace {
bool await_working(LocalAdapterSession& session,CoreLoop& core,std::size_t count,LogicalTime at){
    const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(3);
    while(std::chrono::steady_clock::now()<until){
        if(session.process_reports(at)!=Error::None)return false;
        std::size_t working=0;for(const auto& [id,order]:core.state().orders){(void)id;working+=order.execution()==ExecutionState::Working;}
        if(working==count)return true;std::this_thread::yield();
    }return false;
}
SubmitOrder request(const CoreLoop& core,Side side=Side::Buy,Ticks price=499){return {{core.scope().account,core.scope().contract.instrument,core.scope().day,side,Offset::Open,price,1},
    {core.scope().day,9*3600+60,core.scope().day},{},core.config().strategy};}
void apply_outcome(CoreLoop& core,const LocalAdapterOutcome& outcome,LogicalTime at){
    for(const auto& event:outcome.reply.reports)CHECK_EQ(core.step(event,at).error,Error::None);
}
}
TEST_CASE(ADAPTER_core_accepted_identity_crosses_thread_and_independent_peer_query_aligns){
    const auto c=example::quote_runner_config();CoreLoop core(c.core);CHECK_EQ(core.step(ReferenceMark{500},0).error,Error::None);
    LocalAdapterSession session(core,{c.peer});const auto sent=session.submit(request(core),1);
    CHECK(sent.core.applied);CHECK(sent.core.order);CHECK_EQ(sent.dispatch,Error::None);CHECK(sent.permit);
    const auto reservation=core.state().orders.at(*sent.core.order).reservation();CHECK(reservation.valid());
    session.start();const auto& query=session.finish();CHECK(query.snapshot);CHECK_EQ(query.snapshot->orders.size(),1U);
    auto outcome=session.receive();CHECK_EQ(outcome.result,QueueResult::Success);CHECK(outcome.value->called);CHECK_EQ(outcome.value->order,*sent.core.order);CHECK_EQ(outcome.value->permit,*sent.permit);
    apply_outcome(core,*outcome.value,2);CHECK_EQ(session.receive().result,QueueResult::Empty);
    CHECK_EQ(compare_local_query_with_core(query,query.snapshot->query,c.peer,core).status,LocalQueryStatus::Complete);
    CHECK(session.gate_snapshot().quiesced);CHECK(core.invariant());
}
TEST_CASE(ADAPTER_kill_before_worker_claim_does_not_call_or_release_core_reservation){
    const auto c=example::quote_runner_config();CoreLoop core(c.core);core.step(ReferenceMark{500},0);LocalAdapterSession session(core,{c.peer});
    const auto sent=session.submit(request(core),1);CHECK_EQ(sent.dispatch,Error::None);CHECK_EQ(session.kill(2).error,Error::None);
    const auto& query=session.finish();CHECK(query.snapshot);CHECK(query.snapshot->orders.empty());
    auto outcome=session.receive();CHECK(outcome.value);CHECK(!outcome.value->called);CHECK_EQ(outcome.value->error,Error::Halted);CHECK(outcome.value->reply.reports.empty());
    CHECK_EQ(core.state().orders.at(*sent.core.order).parts().possible,1);CHECK_EQ(core.state().mode,CoreMode::Killed);CHECK(session.gate_snapshot().quiesced);
}
TEST_CASE(ADAPTER_outbound_capacity_failure_keeps_both_core_reservations_and_closes_gate){
    const auto c=example::quote_runner_config();CoreLoop core(c.core);core.step(ReferenceMark{500},0);LocalAdapterSession session(core,{c.peer,1,4});
    const auto first=session.submit(request(core),1),second=session.submit(request(core,Side::Sell,501),2);
    CHECK_EQ(first.dispatch,Error::None);CHECK(second.core.applied);CHECK_EQ(second.dispatch,Error::Capacity);CHECK(!session.gate_snapshot().open);
    const auto& query=session.finish();CHECK(query.snapshot->orders.empty());CHECK_EQ(core.state().orders.size(),2U);
    for(const auto& [id,order]:core.state().orders){(void)id;CHECK_EQ(order.parts().possible,1);}
    CHECK(session.gate_snapshot().quiesced);
}
TEST_CASE(ADAPTER_report_overflow_stops_next_claim_and_drain_cannot_reopen){
    const auto c=example::quote_runner_config();CoreLoop core(c.core);core.step(ReferenceMark{500},0);LocalAdapterSession session(core,{c.peer,4,1});
    for(int i=0;i<3;++i)CHECK_EQ(session.submit(request(core,Side::Buy,499-i),static_cast<LogicalTime>(i+1)).dispatch,Error::None);
    const auto& query=session.finish();CHECK_EQ(query.snapshot->orders.size(),2U);CHECK(session.gate_snapshot().loss.value>=1);CHECK(!session.gate_snapshot().open);
    auto outcome=session.receive();CHECK(outcome.value);CHECK(outcome.value->called);apply_outcome(core,*outcome.value,4);
    CHECK_EQ(session.receive().result,QueueResult::Empty);CHECK(!session.gate_snapshot().open);CHECK_EQ(core.state().orders.size(),3U);
    CHECK(compare_local_query_with_core(query,query.snapshot->query,c.peer,core).status!=LocalQueryStatus::Complete);
    for(const auto& [id,order]:core.state().orders){(void)id;CHECK_EQ(order.parts().held(),1);}CHECK(core.invariant());
}
TEST_CASE(ADAPTER_duplicate_client_and_core_rejection_never_create_extra_peer_orders){
    const auto c=example::quote_runner_config();CoreLoop core(c.core);core.step(ReferenceMark{500},0);LocalAdapterSession session(core,{c.peer});
    auto input=request(core);input.client_id=ClientOrderId{"adapter.test","one"};CHECK_EQ(session.submit(input,1).dispatch,Error::None);
    const auto duplicate=session.submit(input,2);CHECK_EQ(duplicate.core.error,Error::Duplicate);CHECK(!duplicate.permit);
    input.client_id.reset();input.intent.quantity=11;const auto invalid=session.submit(input,3);CHECK(!invalid.core.applied);CHECK(!invalid.permit);
    CHECK_EQ(session.finish().snapshot->orders.size(),1U);
}
TEST_CASE(ADAPTER_foreign_thread_cannot_write_core_and_profile_mismatch_is_rejected){
    const auto c=example::quote_runner_config();CoreLoop core(c.core);core.step(ReferenceMark{500},0);
    auto wrong=c.peer;wrong.account="foreign";CHECK_THROWS((LocalAdapterSession(core,{wrong})),std::invalid_argument);
    LocalAdapterSession session(core,{c.peer});const auto input=request(core);
    auto foreign=std::async(std::launch::async,[&]{try{session.submit(input,1);}catch(const std::logic_error&){return true;}return false;});
    CHECK(foreign.get());CHECK(core.state().orders.empty());CHECK(session.finish().snapshot->orders.empty());
}
TEST_CASE(ADAPTER_one_core_has_one_session_until_destruction_even_after_finish){
    const auto c=example::quote_runner_config();CoreLoop core(c.core);
    {
        LocalAdapterSession session(core,{c.peer});
        CHECK_THROWS((LocalAdapterSession(core,{c.peer})),std::logic_error);
        CHECK(session.finish().snapshot->orders.empty());
        CHECK_THROWS((LocalAdapterSession(core,{c.peer})),std::logic_error);
    }
    LocalAdapterSession replacement(core,{c.peer});CHECK(replacement.finish().snapshot->orders.empty());
}
TEST_CASE(ADAPTER_failed_constructor_releases_lease_before_valid_session_can_attach){
    const auto c=example::quote_runner_config();CoreLoop core(c.core);auto invalid=c.peer;invalid.fee_per_lot+=1;
    CHECK_THROWS((LocalAdapterSession(core,{invalid})),std::invalid_argument);
    LocalAdapterSession session(core,{c.peer});CHECK(session.finish().snapshot->orders.empty());
}
TEST_CASE(ADAPTER_foreign_thread_second_session_is_rejected_before_reading_core_state){
    const auto c=example::quote_runner_config();CoreLoop core(c.core);LocalAdapterSession session(core,{c.peer});
    auto second=std::async(std::launch::async,[&]{try{LocalAdapterSession duplicate(core,{c.peer});}catch(const std::logic_error&){return true;}return false;});
    CHECK(second.get());CHECK(session.finish().snapshot->orders.empty());
}
TEST_CASE(ADAPTER_failed_report_is_retained_until_explicit_retry_and_blocks_new_submission){
    const auto c=example::quote_runner_config();CoreLoop core(c.core);core.step(ReferenceMark{500},0);LocalAdapterSession session(core,{c.peer});
    const auto sent=session.submit(request(core),1);CHECK_EQ(sent.dispatch,Error::None);session.start();core.fail_next_commit_for_test();
    const auto deadline=std::chrono::steady_clock::now()+std::chrono::seconds(3);Error failure=Error::None;
    while(std::chrono::steady_clock::now()<deadline){failure=session.process_reports(2);if(failure!=Error::None)break;std::this_thread::yield();}
    CHECK_EQ(failure,Error::InjectedFailure);CHECK_EQ(session.retained_reports(),1U);
    CHECK((session.gate_snapshot().reasons&static_cast<std::uint32_t>(LocalGateReason::EvidenceFailure))!=0);
    const auto state=core.canonical_state();CHECK_EQ(session.process_reports(3),Error::Blocked);CHECK_EQ(core.canonical_state(),state);
    CHECK_THROWS(session.receive(),std::logic_error);CHECK_EQ(session.submit(request(core,Side::Sell,501),3).dispatch,Error::Halted);
    CHECK_EQ(core.state().orders.size(),1U);CHECK_EQ(core.state().orders.at(*sent.core.order).execution(),ExecutionState::Created);
    const auto& query=session.finish();CHECK_EQ(session.retry_reports(4),Error::None);CHECK_EQ(session.retained_reports(),0U);
    CHECK_EQ(compare_local_query_with_core(query,query.snapshot->query,c.peer,core).status,LocalQueryStatus::Complete);
    const auto after=core.canonical_state();CHECK_EQ(session.retry_reports(5),Error::None);CHECK_EQ(core.canonical_state(),after);CHECK(!session.gate_snapshot().open);
}
TEST_CASE(ADAPTER_failed_first_outcome_cannot_be_skipped_to_apply_second){
    const auto c=example::quote_runner_config();CoreLoop core(c.core);core.step(ReferenceMark{500},0);LocalAdapterSession session(core,{c.peer});
    const auto first=session.submit(request(core),1),second=session.submit(request(core,Side::Buy,498),2);CHECK_EQ(first.dispatch,Error::None);CHECK_EQ(second.dispatch,Error::None);
    const auto& query=session.finish();core.fail_next_commit_for_test();CHECK_EQ(session.process_reports(3),Error::InjectedFailure);
    CHECK_EQ(session.process_reports(4),Error::Blocked);CHECK_EQ(core.state().orders.at(*second.core.order).execution(),ExecutionState::Created);
    CHECK_EQ(session.retry_reports(5),Error::None);CHECK_EQ(core.state().orders.at(*first.core.order).execution(),ExecutionState::Working);
    CHECK_EQ(session.process_reports(6),Error::None);CHECK_EQ(core.state().orders.at(*second.core.order).execution(),ExecutionState::Working);
    CHECK_EQ(compare_local_query_with_core(query,query.snapshot->query,c.peer,core).status,LocalQueryStatus::Complete);
    CHECK(core.invariant());CHECK_EQ(session.retained_reports(),0U);
}
TEST_CASE(ADAPTER_kill_blocks_new_orders_but_reserved_cancel_lane_reaches_peer_and_releases_only_on_terminal){
    const auto c=example::quote_runner_config();CoreLoop core(c.core);core.step(ReferenceMark{500},0);LocalAdapterSession session(core,{c.peer});
    const auto sent=session.submit(request(core),1);CHECK_EQ(sent.dispatch,Error::None);session.start();CHECK(await_working(session,core,1,2));
    CHECK_EQ(session.kill(3).error,Error::None);CHECK_EQ(session.submit(request(core,Side::Sell,501),4).dispatch,Error::Halted);
    const auto canceled=session.cancel(*sent.core.order,4);CHECK_EQ(canceled.dispatch,Error::None);CHECK(canceled.core.cancel);
    CHECK_EQ(core.state().orders.at(*sent.core.order).parts().possible,1);
    const auto& query=session.finish();CHECK_EQ(query.snapshot->orders[0].status,SimOrderStatus::Canceled);
    CHECK_EQ(session.process_reports(5),Error::None);CHECK_EQ(core.state().orders.at(*sent.core.order).parts().held(),0);
    CHECK_EQ(core.state().mode,CoreMode::Killed);CHECK(!session.gate_snapshot().open);
    CHECK_EQ(compare_local_query_with_core(query,query.snapshot->query,c.peer,core).status,LocalQueryStatus::Complete);
}
TEST_CASE(ADAPTER_cancel_report_failure_retains_ack_and_terminal_until_explicit_retry){
    const auto c=example::quote_runner_config();CoreLoop core(c.core);core.step(ReferenceMark{500},0);LocalAdapterSession session(core,{c.peer});
    const auto sent=session.submit(request(core),1);session.start();CHECK(await_working(session,core,1,2));session.kill(3);
    CHECK_EQ(session.cancel(*sent.core.order,4).dispatch,Error::None);const auto& query=session.finish();core.fail_next_commit_for_test();
    CHECK_EQ(session.process_reports(5),Error::InjectedFailure);CHECK_EQ(session.retained_reports(),2U);
    CHECK_EQ(core.state().orders.at(*sent.core.order).parts().possible,1);CHECK_EQ(session.process_reports(6),Error::Blocked);
    CHECK_EQ(session.retry_reports(7),Error::None);CHECK_EQ(session.retained_reports(),0U);CHECK_EQ(core.state().orders.at(*sent.core.order).parts().held(),0);
    CHECK_EQ(core.state().orders.at(*sent.core.order).cancel(),CancelState::Succeeded);
    CHECK_EQ(compare_local_query_with_core(query,query.snapshot->query,c.peer,core).status,LocalQueryStatus::Complete);CHECK(!session.gate_snapshot().open);
}
TEST_CASE(ADAPTER_cancel_budget_rejection_does_not_mark_second_order_cancel_requested){
    const auto c=example::quote_runner_config();CoreLoop core(c.core);core.step(ReferenceMark{500},0);LocalAdapterConfig config{c.peer};config.cancel_requests=1;
    LocalAdapterSession session(core,config);const auto first=session.submit(request(core),1),second=session.submit(request(core,Side::Buy,498),2);
    session.start();CHECK(await_working(session,core,2,3));session.kill(4);CHECK_EQ(session.cancel(*first.core.order,5).dispatch,Error::None);
    CHECK_EQ(session.cancel(*second.core.order,6).dispatch,Error::Capacity);CHECK_EQ(core.state().orders.at(*second.core.order).cancel(),CancelState::None);
    const auto& query=session.finish();CHECK_EQ(session.process_reports(7),Error::None);
    CHECK_EQ(core.state().orders.at(*first.core.order).parts().held(),0);CHECK_EQ(core.state().orders.at(*second.core.order).parts().possible,1);
    CHECK_EQ(compare_local_query_with_core(query,query.snapshot->query,c.peer,core).status,LocalQueryStatus::Complete);
}
TEST_CASE(ADAPTER_cancel_cannot_overtake_unconfirmed_new_order_or_run_after_finish){
    const auto c=example::quote_runner_config();CoreLoop core(c.core);core.step(ReferenceMark{500},0);LocalAdapterSession session(core,{c.peer});
    const auto sent=session.submit(request(core),1);const auto before=core.canonical_state();CHECK_EQ(session.cancel(*sent.core.order,2).dispatch,Error::Blocked);
    CHECK_EQ(core.canonical_state(),before);CHECK_EQ(session.cancel({7,999},2).dispatch,Error::UnknownOrder);
    session.finish();CHECK_EQ(session.cancel(*sent.core.order,3).dispatch,Error::Halted);
}
