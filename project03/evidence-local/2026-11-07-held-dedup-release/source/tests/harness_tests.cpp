#include "fixture.hpp"
using namespace fixture;

TEST_CASE(HARNESS_equal_time_preserves_insertion_order) {
    StepHarness h(config()); CHECK_EQ(h.schedule(0,ReferenceMark{500}),Error::None);
    CHECK_EQ(h.schedule(1,KillRequested{}),Error::None);
    CHECK_EQ(h.schedule(1,SubmitOrder{intent(),session()}),Error::None);
    CHECK_EQ(h.run(),Error::None); CHECK_EQ(h.trace().size(),3U);
    CHECK_EQ(h.trace()[2].result.error,Error::Halted); CHECK(h.core().state().orders.empty());
}
TEST_CASE(HARNESS_same_ordered_inputs_same_business_states) {
    StepHarness a(config()),b(config());
    const OrderId expected{7,1};
    const std::array<std::pair<LogicalTime,BusinessEvent>,6> inputs{{{0,ReferenceMark{500}},
        {1,SubmitOrder{intent(),session()}},{2,CumulativeReport{header(expected),2}},
        {3,trade(expected,"a")},{4,TerminalReport{header(expected),{TerminalKind::Canceled,2}}},
        {5,trade(expected,"b")}}};
    for(const auto& [at,input]:inputs) { CHECK_EQ(a.schedule(at,input),Error::None); CHECK_EQ(b.schedule(at,input),Error::None); }
    CHECK_EQ(a.run(),Error::None); CHECK_EQ(b.run(),Error::None);
    CHECK_EQ(a.trace().size(),b.trace().size());
    for(std::size_t i=0;i<a.trace().size();++i) CHECK_EQ(a.trace()[i].canonical_state,b.trace()[i].canonical_state);
    CHECK_EQ(a.core().state().positions.booked()[0],2);
}
TEST_CASE(HARNESS_bounded_schedule_and_past_time_rejection) {
    StepHarness h(config(),2); CHECK_EQ(h.schedule(1,ReferenceMark{500}),Error::None);
    CHECK_EQ(h.schedule(2,KillRequested{}),Error::None);
    CHECK_EQ(h.schedule(3,ReferenceMark{500}),Error::Capacity); CHECK_EQ(h.run(),Error::None);
    CHECK_EQ(h.schedule(1,ReferenceMark{500}),Error::OutOfOrderTime);
    CHECK_EQ(h.schedule(3,ReferenceMark{500}),Error::Capacity); CHECK_EQ(h.trace().size(),2U);
}
TEST_CASE(HARNESS_core_rejects_clock_regression_without_financial_change) {
    CoreLoop core(config()); CHECK_EQ(core.step(ReferenceMark{500},10).error,Error::None);
    const auto before=core.canonical_state(); CHECK_EQ(core.step(ReferenceMark{501},9).error,Error::OutOfOrderTime);
    CHECK_EQ(core.canonical_state(),before);
}
TEST_CASE(HARNESS_failed_evidence_input_is_retained_with_result) {
    StepHarness h(config()); auto bad=header({7,999});
    CHECK_EQ(h.schedule(1,CumulativeReport{bad,1}),Error::None); CHECK_EQ(h.run(),Error::None);
    CHECK_EQ(h.trace()[0].result.error,Error::UnknownOrder);
    CHECK_EQ(std::get<CumulativeReport>(h.trace()[0].input).header.order,bad.order);
    CHECK(h.core().state().unknown_exposure);
}
TEST_CASE(HARNESS_summary_distinguishes_hidden_cumulative_versions) {
    Fixture a,b; const auto aid=a.submit(),bid=b.submit();
    CHECK_EQ(a.apply(CumulativeReport{header(aid,2),2}).error,Error::None);
    CHECK_EQ(b.apply(CumulativeReport{header(bid,3),2}).error,Error::None);
    CHECK_EQ(a.order(aid).parts(),b.order(bid).parts());
    CHECK(a.core.canonical_state()!=b.core.canonical_state());
}
