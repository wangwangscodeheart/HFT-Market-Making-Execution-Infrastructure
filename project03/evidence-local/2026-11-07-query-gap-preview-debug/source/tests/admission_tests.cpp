#include "fixture.hpp"
#include "../examples/demo_scenario.hpp"
using namespace fixture;

TEST_CASE(ADMISSION_rejected_local_input_does_not_consume_business_sequence_or_clock) {
    CoreLoop core(config());const auto before=core.canonical_state();
    const auto bad=core.step(ReferenceMark{0},1000);
    CHECK_EQ(bad.error,Error::InvalidInput);CHECK_EQ(bad.ingress_sequence,1U);
    CHECK_EQ(bad.core_sequence,0U);CHECK(!bad.command.valid());CHECK(!bad.applied);
    CHECK_EQ(bad.admission,InputAdmission::RejectedInput);CHECK_EQ(core.canonical_state(),before);
    const auto mark=core.step(ReferenceMark{500},1);
    CHECK_EQ(mark.error,Error::None);CHECK_EQ(mark.ingress_sequence,2U);CHECK_EQ(mark.core_sequence,1U);
    CHECK_EQ(mark.command.serial,1U);CHECK_EQ(mark.admission,InputAdmission::BusinessInput);
}

TEST_CASE(ADMISSION_untrusted_trade_becomes_local_rejection_fact_not_booking) {
    const auto scenario=example::standard_filled_order();CoreLoop core(scenario.config);
    CHECK_EQ(core.step(scenario.inputs[0],0).error,Error::None);
    CHECK_EQ(core.step(scenario.inputs[1],1).error,Error::None);
    auto bad=std::get<TradeReport>(scenario.inputs[3]);bad.stamp.origin=EvidenceOrigin::Unverified;
    const auto result=core.step(bad,2);
    CHECK_EQ(result.error,Error::UntrustedEvidence);
    CHECK_EQ(result.admission,InputAdmission::RejectedEvidenceFact);
    CHECK_EQ(result.ingress_sequence,3U);CHECK_EQ(result.core_sequence,3U);CHECK(result.applied);
    CHECK_EQ(core.state().mode,CoreMode::ConflictHalted);
    CHECK(core.state().executions.records().empty());CHECK_EQ(core.state().positions.booked()[0],0);
    CHECK_EQ(core.state().quarantine.size(),1U);CHECK(core.state().quarantine.back().trade.has_value());
    CHECK_EQ(core.state().quarantine.back().ingress_sequence,result.ingress_sequence);
    CHECK_EQ(core.state().quarantine.back().core_sequence,result.core_sequence);
    CHECK_EQ(core.state().funds.view().local_required,1020);CHECK(core.invariant());
}

TEST_CASE(ADMISSION_wrong_identity_request_does_not_use_next_order_or_command_ID) {
    CoreLoop core(config());CHECK_EQ(core.step(ReferenceMark{500},0).core_sequence,1U);
    auto i=intent();i.account="foreign.account";
    const auto rejected=core.step(SubmitOrder{i,session()},100);
    CHECK_EQ(rejected.error,Error::WrongIdentity);CHECK_EQ(rejected.core_sequence,0U);
    const auto accepted=core.step(SubmitOrder{intent(),session()},1);
    CHECK_EQ(accepted.error,Error::None);CHECK(accepted.order.has_value());
    CHECK_EQ(accepted.order->serial,1U);CHECK_EQ(accepted.command.serial,2U);
    CHECK_EQ(accepted.core_sequence,2U);CHECK_EQ(accepted.ingress_sequence,3U);
}

TEST_CASE(ADMISSION_risk_rejection_is_an_admitted_business_decision) {
    auto c=config();c.risk.local_funds=200;CoreLoop core(c);
    CHECK_EQ(core.step(ReferenceMark{500},0).error,Error::None);
    const auto rejected=core.step(SubmitOrder{intent(),session()},1);
    CHECK_EQ(rejected.error,Error::InsufficientFunds);
    CHECK_EQ(rejected.admission,InputAdmission::BusinessInput);CHECK_EQ(rejected.core_sequence,2U);
    CHECK(rejected.command.valid());CHECK(!rejected.applied);CHECK(core.state().orders.empty());
}

TEST_CASE(ADMISSION_valid_duplicate_remains_ordered_without_double_booking) {
    const auto scenario=example::standard_filled_order();CoreLoop core(scenario.config);
    CHECK_EQ(core.step(scenario.inputs[0],0).error,Error::None);
    CHECK_EQ(core.step(scenario.inputs[1],1).error,Error::None);
    CHECK_EQ(core.step(scenario.inputs[3],2).error,Error::None);
    const auto duplicate=core.step(scenario.inputs[3],3);
    CHECK_EQ(duplicate.error,Error::Duplicate);CHECK_EQ(duplicate.admission,InputAdmission::BusinessInput);
    CHECK_EQ(duplicate.core_sequence,4U);CHECK(!duplicate.applied);
    CHECK_EQ(core.state().positions.booked()[0],2);CHECK_EQ(core.state().executions.records().size(),1U);
}

TEST_CASE(ADMISSION_conflicting_trade_key_marks_original_and_related_order) {
    Fixture f;const auto first=f.submit();const auto second=f.submit();
    const auto original=trade(first,"shared-key");CHECK_EQ(f.apply(original).error,Error::None);
    auto conflict=original;conflict.trade.order=second;
    const auto result=f.apply(conflict);
    CHECK_EQ(result.error,Error::EvidenceConflict);CHECK_EQ(result.admission,InputAdmission::RejectedEvidenceFact);
    CHECK_EQ(f.order(first).consistency(),Consistency::Conflict);
    CHECK_EQ(f.order(second).consistency(),Consistency::Conflict);
    CHECK_EQ(f.order(first).booked(),1);CHECK_EQ(f.order(second).booked(),0);
    CHECK_EQ(f.core.state().executions.records().at(original.trade.key).trade,original.trade);
}

TEST_CASE(ADMISSION_harness_retains_raw_rejection_and_independent_identifiers) {
    StepHarness harness(config());
    CHECK_EQ(harness.schedule(0,ReferenceMark{0}),Error::None);
    CHECK_EQ(harness.schedule(1,ReferenceMark{500}),Error::None);
    CHECK_EQ(harness.run(),Error::None);CHECK_EQ(harness.trace().size(),2U);
    CHECK_EQ(std::get<ReferenceMark>(harness.trace()[0].input).price,0);
    CHECK_EQ(harness.trace()[0].result.core_sequence,0U);CHECK_EQ(harness.trace()[0].result.ingress_sequence,1U);
    CHECK_EQ(harness.trace()[1].result.core_sequence,1U);CHECK_EQ(harness.trace()[1].result.ingress_sequence,2U);
}

TEST_CASE(ADMISSION_bad_observation_and_unknown_operation_cannot_advance_clock) {
    CoreLoop core(config());auto observation=position_observation();observation.metadata.source="untrusted.source";
    const auto rejected=core.step(ObservePosition{observation},100);
    CHECK_EQ(rejected.error,Error::InvalidInput);CHECK_EQ(rejected.core_sequence,0U);
    CHECK_EQ(rejected.admission,InputAdmission::RejectedInput);
    const auto operation=core.step(RequestFutureOperation{static_cast<FutureOperation>(99)},100);
    CHECK_EQ(operation.error,Error::InvalidInput);CHECK_EQ(operation.core_sequence,0U);
    CHECK_EQ(core.step(ReferenceMark{500},0).core_sequence,1U);CHECK(core.invariant());
}
