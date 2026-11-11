#include "fixture.hpp"
using namespace fixture;
namespace {
CoreConfig simulator(){auto c=config();c.trade_evidence_mode=TradeEvidenceMode::Simulator;return c;}
LocalRuntimeFault fault(const CoreLoop& core,RuntimeFaultState state={2,2,1}){return {core.scope(),core.config().instance,state};}
}
TEST_CASE(RUNTIME_FAULT_validates_local_profile_scope_and_bounded_reason_mask){
    CoreLoop fixture_core(config());CHECK_EQ(fixture_core.step(fault(fixture_core),1).error,Error::Blocked);
    for(int variant=0;variant<6;++variant){CoreLoop core(simulator());auto event=fault(core);
        if(variant==0)event.scope.account="foreign";if(variant==1)event.instance=8;if(variant==2)event.fault.reasons=0;
        if(variant==3)event.fault.reasons=128;if(variant==4)event.fault.latch_version=0;if(variant==5)event.fault.loss_generation=0;
        CHECK(core.step(event,1).error!=Error::None);CHECK_EQ(core.state().runtime_fault,RuntimeFaultState{});CHECK_EQ(core.state().mode,CoreMode::Ready);
    }
}
TEST_CASE(RUNTIME_FAULT_monotonic_state_preserves_stops_and_replays_deterministically){
    CoreLoop core(simulator()),replay(simulator());
    const std::vector<BusinessEvent> events{fault(core,{64,2,0}),fault(core,{66,3,1}),fault(core,{66,3,2}),KillRequested{}};
    LogicalTime at=0;for(const auto& event:events){++at;CHECK_EQ(core.step(event,at).error,Error::None);CHECK_EQ(replay.step(event,at).error,Error::None);}
    CHECK_EQ(core.canonical_state(),replay.canonical_state());CHECK_EQ(core.state().mode,CoreMode::ConflictHalted);CHECK(core.state().unknown_exposure);
    CHECK_EQ(core.step(fault(core,{66,3,2}),5).error,Error::Duplicate);
    for(const auto state:{RuntimeFaultState{64,2,0},RuntimeFaultState{2,4,2},RuntimeFaultState{70,3,2}}){
        CHECK_EQ(core.step(fault(core,state),6).error,Error::EvidenceConflict);CHECK_EQ(core.state().runtime_fault,(RuntimeFaultState{66,3,2}));
    }CHECK(core.invariant());
}
TEST_CASE(RUNTIME_FAULT_failed_commit_is_atomic_and_killed_mode_remains_killed){
    CoreLoop core(simulator());core.fail_next_commit_for_test();const auto event=fault(core);
    CHECK_EQ(core.step(event,1).error,Error::InjectedFailure);CHECK_EQ(core.state().runtime_fault,RuntimeFaultState{});CHECK_EQ(core.state().mode,CoreMode::Ready);CHECK(!core.state().unknown_exposure);
    CHECK_EQ(core.step(KillRequested{},2).error,Error::None);CHECK_EQ(core.step(event,3).error,Error::None);
    CHECK_EQ(core.state().mode,CoreMode::Killed);CHECK(core.state().unknown_exposure);CHECK(core.invariant());
}
TEST_CASE(RUNTIME_FAULT_heartbeat_stop_is_sticky_without_fabricating_report_loss){
    CoreLoop core(simulator());CHECK_EQ(core.step(fault(core,{8,2,0}),1).error,Error::None);
    CHECK_EQ(core.state().mode,CoreMode::ConflictHalted);CHECK(!core.state().unknown_exposure);CHECK_EQ(core.state().runtime_fault.loss_generation,0U);
    CHECK_EQ(core.step(KillRequested{},2).error,Error::None);CHECK_EQ(core.state().mode,CoreMode::ConflictHalted);CHECK(core.invariant());
}
