#include "fixture.hpp"
#include "../examples/demo_scenario.hpp"
using namespace fixture;

TEST_CASE(INPUT_invalid_channel_cannot_book_standard_fill) {
    const auto scenario=example::standard_filled_order();
    for(const int invalid:{-1,2,99}){
        Fixture f(scenario.config);
        CHECK_EQ(f.apply(scenario.inputs[1]).error,Error::None);
        CHECK_EQ(f.apply(scenario.inputs[2]).error,Error::None);
        auto report=std::get<TradeReport>(scenario.inputs[3]);
        report.channel=static_cast<Channel>(invalid);
        const auto before=f.core.state().funds.view().local_required;
        CHECK_EQ(f.apply(report).error,Error::InvalidInput);
        CHECK(f.core.state().executions.records().empty());
        CHECK_EQ(f.order(scenario.order).booked(),0);
        CHECK_EQ(f.core.state().positions.booked()[0],0);
        CHECK_EQ(f.core.state().funds.view().local_required,before);
        CHECK_EQ(f.core.state().quarantine.size(),1U);
        CHECK_EQ(f.core.state().mode,CoreMode::ConflictHalted);
    }
}

TEST_CASE(INPUT_invalid_channel_cannot_bypass_validation_via_duplicate) {
    const auto scenario=example::standard_filled_order();Fixture f(scenario.config);
    CHECK_EQ(f.apply(scenario.inputs[1]).error,Error::None);
    CHECK_EQ(f.apply(scenario.inputs[3]).error,Error::None);
    auto report=std::get<TradeReport>(scenario.inputs[3]);report.channel=static_cast<Channel>(99);
    CHECK_EQ(f.apply(report).error,Error::InvalidInput);
    CHECK_EQ(f.order(scenario.order).booked(),2);
    CHECK_EQ(f.core.state().executions.records().size(),1U);
    CHECK_EQ(f.core.state().mode,CoreMode::ConflictHalted);
}

TEST_CASE(TRANSACTION_failed_standard_fill_does_not_leave_ghost_dedup_or_position) {
    const auto scenario=example::standard_filled_order();Fixture f(scenario.config);
    CHECK_EQ(f.apply(scenario.inputs[1]).error,Error::None);
    CHECK_EQ(f.apply(scenario.inputs[2]).error,Error::None);
    const auto before=f.core.state().funds.resources();
    f.core.fail_next_commit_for_test();
    const auto failed=f.apply(scenario.inputs[3]);
    CHECK_EQ(failed.error,Error::InjectedFailure);CHECK(!failed.applied);
    CHECK_EQ(f.order(scenario.order).execution(),ExecutionState::Working);
    CHECK_EQ(f.order(scenario.order).parts(),(QuantityParts{0,0,10,0,0}));
    CHECK(f.core.state().executions.records().empty());
    CHECK_EQ(f.core.state().positions.booked(),(std::array<Lots,4>{}));
    CHECK_EQ(f.core.state().funds.resources(),before);
    CHECK_EQ(f.apply(scenario.inputs[3]).error,Error::None);
    CHECK_EQ(f.order(scenario.order).booked(),2);
    CHECK_EQ(f.core.state().executions.records().size(),1U);
    CHECK_EQ(f.core.state().positions.booked()[0],2);
}

TEST_CASE(TRANSACTION_failed_terminal_does_not_release_resources_until_retry) {
    const auto scenario=example::cancellation_race();Fixture f(scenario.config);
    CHECK_EQ(f.apply(scenario.inputs[1]).error,Error::None);
    CHECK_EQ(f.apply(scenario.inputs[2]).error,Error::None);
    CHECK_EQ(f.apply(scenario.inputs[3]).error,Error::None);
    const auto before=f.core.state().funds.resources();
    f.core.fail_next_commit_for_test();
    const auto failed=f.apply(scenario.inputs[4]);
    CHECK_EQ(failed.error,Error::InjectedFailure);CHECK(!failed.applied);
    CHECK_EQ(f.order(scenario.order).parts(),(QuantityParts{1,0,2,0,0}));
    CHECK_EQ(f.core.state().funds.resources(),before);
    CHECK_EQ(f.core.state().positions.booked()[0],1);
    CHECK_EQ(f.apply(scenario.inputs[4]).error,Error::None);
    CHECK_EQ(f.order(scenario.order).parts(),(QuantityParts{1,1,0,1,0}));
    CHECK_EQ(f.core.state().funds.view().local_required,204);
}

TEST_CASE(TRANSACTION_duplicate_does_not_consume_next_commit_failure_seam) {
    const auto scenario=example::cancellation_race();Fixture f(scenario.config);
    CHECK_EQ(f.apply(scenario.inputs[1]).error,Error::None);
    CHECK_EQ(f.apply(scenario.inputs[2]).error,Error::None);
    f.core.fail_next_commit_for_test();
    CHECK_EQ(f.apply(scenario.inputs[2]).error,Error::Duplicate);
    CHECK_EQ(f.apply(scenario.inputs[5]).error,Error::InjectedFailure);
    CHECK_EQ(f.order(scenario.order).booked(),1);
    CHECK_EQ(f.core.state().executions.records().size(),1U);
    CHECK_EQ(f.apply(scenario.inputs[5]).error,Error::None);
    CHECK_EQ(f.order(scenario.order).booked(),2);
}
