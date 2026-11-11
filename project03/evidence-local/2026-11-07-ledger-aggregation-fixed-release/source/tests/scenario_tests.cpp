#include "fixture.hpp"
#include "../examples/demo_scenario.hpp"
using namespace fixture;

TEST_CASE(STANDARD_A1a_accepted_order_fills_in_source_proportions) {
    const auto scenario=example::standard_filled_order();
    StepHarness harness(scenario.config);
    // Source: official A.1.a normal branch; 10000 = 2000 + 1000 + 7000.
    constexpr std::array<Lots,6> booked{0,0,0,2,3,10};
    constexpr std::array<Lots,6> remaining{0,10,10,8,7,0};
    constexpr std::array<ExecutionState,5> execution{ExecutionState::Created,ExecutionState::Working,
        ExecutionState::PartiallyFilled,ExecutionState::PartiallyFilled,ExecutionState::Filled};
    for(std::size_t i=0;i<scenario.inputs.size();++i){
        CHECK_EQ(harness.schedule(static_cast<LogicalTime>(i),scenario.inputs[i]),Error::None);
        CHECK_EQ(harness.run(),Error::None);
        CHECK_EQ(harness.trace().back().result.error,Error::None);
        CHECK(harness.trace().back().result.applied);
        CHECK(harness.core().invariant());
        const auto& state=harness.core().state();
        CHECK_EQ(state.positions.booked(),(std::array<Lots,4>{booked[i],0,0,0}));
        CHECK_EQ(state.positions.frozen(),(std::array<Lots,4>{}));
        if(i==0){CHECK(state.orders.empty());continue;}
        const auto& order=state.orders.at(scenario.order);
        CHECK_EQ(order.parts(),(QuantityParts{booked[i],0,remaining[i],0,0}));
        CHECK_EQ(order.execution(),execution[i-1]);
        CHECK_EQ(order.send(),SendState::NotImplemented);
        CHECK_EQ(order.cancel(),CancelState::None);
        CHECK_EQ(order.consistency(),Consistency::InSync);
        CHECK_EQ(state.funds.view().local_required,1020);
        CHECK_EQ(harness.core().risk_view().max_net,10);
    }
    CHECK_EQ(harness.core().state().executions.records().size(),3U);
    CHECK_EQ(harness.core().risk_view().booked_net,10);
    CHECK_EQ(harness.core().risk_view().min_net,10);
}
