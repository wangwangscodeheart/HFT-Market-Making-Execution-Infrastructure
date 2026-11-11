#include "fixture.hpp"
#include "p03/local_runtime_runner.hpp"
#include "p03/local_query_review.hpp"
#include "../examples/quote_runner_scenario.hpp"
using namespace fixture;
namespace {
LocalRuntimeConfig runtime_config(){const auto c=example::quote_runner_config();return {c.core,{c.peer}};}
SubmitOrder runtime_order(const LocalRuntimeRunner& run){const auto& core=run.core();return {{core.scope().account,core.scope().contract.instrument,core.scope().day,Side::Buy,Offset::Open,499,3},
    {core.scope().day,9*3600+60,core.scope().day},{},core.config().strategy};}
bool pending(LocalRuntimeRunner& run){const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(3);
    while(std::chrono::steady_clock::now()<until){if(run.pending_outcomes()!=0)return true;std::this_thread::yield();}return false;}
}
TEST_CASE(RUNTIME_owned_core_runs_submit_fill_kill_cancel_and_query_through_one_input_path){
    const auto c=runtime_config();LocalRuntimeRunner run(c);
    CHECK_EQ(run.step(ReferenceMark{500},0),Error::None);CHECK_EQ(run.step(RuntimeStart{},1),Error::None);
    CHECK_EQ(run.step(runtime_order(run),2),Error::None);const auto id=*run.trace().back().core_result->order;
    CHECK(pending(run));CHECK_EQ(run.step(RuntimePump{1},3),Error::None);CHECK_EQ(run.core().state().orders.at(id).execution(),ExecutionState::Working);
    CHECK_EQ(run.step(RuntimeFill{id,"runtime-fill",1,499},4),Error::None);CHECK(pending(run));CHECK_EQ(run.step(RuntimePump{1},5),Error::None);
    CHECK_EQ(run.core().state().positions.booked()[0],1);CHECK_EQ(run.step(KillRequested{},6),Error::None);
    CHECK_EQ(run.step(RuntimeCancel{id},7),Error::None);CHECK_EQ(run.step(RuntimeFinish{},8),Error::None);CHECK_EQ(run.step(RuntimePump{8},9),Error::None);
    CHECK_EQ(run.core().state().mode,CoreMode::Killed);CHECK_EQ(run.core().state().orders.at(id).parts().held(),0);
    const auto& query=*run.peer_query();CHECK_EQ(compare_local_query_with_core(query,query.snapshot->query,c.adapter.peer,run.core()).status,LocalQueryStatus::Complete);
    CHECK_EQ(run.trace().size(),10U);for(std::size_t i=0;i<run.trace().size();++i){CHECK(run.trace()[i].completed);CHECK_EQ(run.trace()[i].ordinal,i+1);}
    CHECK(run.core().invariant());
}
TEST_CASE(RUNTIME_capacity_stops_ordinary_inputs_and_reserves_kill_finish_and_pump){
    auto c=runtime_config();c.inputs=2;c.cleanup_inputs=3;LocalRuntimeRunner run(c);run.step(ReferenceMark{500},0);CHECK_EQ(run.step(runtime_order(run),1),Error::None);
    const auto id=*run.trace().back().core_result->order;CHECK_EQ(run.step(RuntimeStart{},2),Error::Capacity);CHECK(run.stopped());CHECK_EQ(run.trace().size(),2U);
    CHECK_EQ(run.step(KillRequested{},3),Error::None);CHECK_EQ(run.step(RuntimeFinish{},4),Error::None);CHECK(run.peer_query()->snapshot->orders.empty());
    CHECK_EQ(run.step(RuntimePump{8},5),Error::Halted);CHECK_EQ(run.core().state().orders.at(id).parts().possible,3);
    CHECK_EQ(run.trace().size(),5U);for(std::size_t i=2;i<5;++i)CHECK(run.trace()[i].cleanup);
    CHECK_EQ(run.step(RuntimePump{8},6),Error::Capacity);CHECK_EQ(run.trace().size(),5U);CHECK(run.core().invariant());
}
TEST_CASE(RUNTIME_finished_worker_still_allows_report_drain_without_new_send_or_false_release){
    const auto c=runtime_config();LocalRuntimeRunner run(c);run.step(ReferenceMark{500},0);run.step(runtime_order(run),1);
    CHECK_EQ(run.step(RuntimeFinish{},2),Error::None);CHECK_EQ(run.step(ReferenceMark{501},3),Error::Halted);
    CHECK_EQ(run.step(RuntimePump{8},3),Error::None);CHECK_EQ(run.step(runtime_order(run),4),Error::Halted);
    CHECK_EQ(run.core().state().orders.begin()->second.parts().possible,3);CHECK_EQ(run.peer_query()->snapshot->orders.size(),1U);
    const auto before=run.core().canonical_state();CHECK_EQ(run.step(RuntimeFinish{},4),Error::None);CHECK_EQ(run.core().canonical_state(),before);
    const auto& query=*run.peer_query();CHECK_EQ(compare_local_query_with_core(query,query.snapshot->query,c.adapter.peer,run.core()).status,LocalQueryStatus::Complete);
}
TEST_CASE(RUNTIME_readonly_core_owner_and_input_bounds_reject_before_recording){
    const auto c=runtime_config();LocalRuntimeRunner run(c);static_assert(std::is_const_v<std::remove_reference_t<decltype(run.core())>>);
    CHECK_EQ(run.step(ReferenceMark{500},2),Error::None);const auto before=run.core().canonical_state();auto input=runtime_order(run);input.intent.account=std::string(129,'x');
    CHECK_EQ(run.step(input,3),Error::InvalidInput);CHECK_EQ(run.step(RuntimeStart{},1),Error::OutOfOrderTime);CHECK_EQ(run.trace().size(),1U);CHECK_EQ(run.core().canonical_state(),before);
    auto foreign=std::async(std::launch::async,[&]{try{run.step(RuntimePump{},3);}catch(const std::logic_error&){return true;}return false;});
    CHECK(foreign.get());CHECK_EQ(run.trace().size(),1U);CHECK_EQ(run.step(RuntimeFinish{},3),Error::None);
}
