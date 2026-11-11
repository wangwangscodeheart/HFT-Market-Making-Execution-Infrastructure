#include "test.hpp"
#include "p03/local_critical_inbox.hpp"
#include <barrier>
#include <future>
using namespace p03;
namespace {
DispatchAuthorizationFact auth(){return {{7,1},{7,2},{7,3},{7,4},{"fixture.run"},
    {"fixture.account",{"SHFE","rb_FIXTURE"},{20261106},1},{1},{{{"fixture.journal"},{1}},{4}},100};}
LocalGateConfig profile(){const auto a=auth();return {7,a.run,a.scope,a.strategy,{1},a.required_prefix,64};}
DispatchPermitId queued(LocalDispatchGate& gate){auto result=gate.enqueue(auth());if(!result.second)throw std::runtime_error("gate setup failed");return *result.second;}
}
TEST_CASE(GATE_five_kill_interleavings_preserve_call_commit_and_quiescence_boundary){
    for(int kill_at=0;kill_at<5;++kill_at){LocalDispatchGate gate(profile());const auto id=queued(gate);bool claimed=false,committed=false;int calls=0;
        for(int stage=0;stage<5;++stage){if(stage==kill_at){gate.close(LocalGateReason::Kill);CHECK(!gate.snapshot().open);if(committed && stage<4)CHECK(!gate.snapshot().quiesced);}
            if(stage==0)claimed=gate.claim(id,1)==Error::None;
            if(stage==1 && claimed)committed=gate.try_commit_call(id,2)==Error::None;
            if(stage==2 && committed)++calls; // Controllable fake SDK entry, deliberately outside gate lock.
            if(stage==3 && committed)CHECK_EQ(gate.call_returned(id),Error::None);
        }
        CHECK_EQ(calls,kill_at<=1?0:1);CHECK(gate.snapshot().quiesced);CHECK_EQ(gate.claim(id,3),kill_at==0?Error::Halted:Error::Duplicate);
    }
}
TEST_CASE(GATE_report_loss_uses_same_boundary_as_kill_and_never_reopens_by_drain){
    for(int committed=0;committed<2;++committed){LocalDispatchGate gate(profile());const auto id=queued(gate);CHECK_EQ(gate.claim(id,1),Error::None);
        if(committed)CHECK_EQ(gate.try_commit_call(id,2),Error::None);
        LocalCriticalInbox<int> inbox(1,gate);CHECK_EQ(inbox.try_deliver(1),QueueResult::Success);CHECK_EQ(inbox.try_deliver(2),QueueResult::Full);
        CHECK(!gate.snapshot().open);CHECK_EQ(gate.snapshot().loss.value,1U);CHECK(!gate.snapshot().quiesced);
        CHECK_EQ(*inbox.try_receive().value,1);CHECK_EQ(inbox.try_receive().result,QueueResult::Empty);CHECK(!gate.snapshot().open);
        if(committed)CHECK_EQ(gate.call_returned(id),Error::None);else CHECK_EQ(gate.try_commit_call(id,3),Error::Halted);
        CHECK(gate.snapshot().quiesced);
    }
}
TEST_CASE(GATE_identity_prefix_expiry_and_duplicate_commands_fail_closed){
    for(int fault=0;fault<4;++fault){LocalDispatchGate gate(profile());auto a=auth();
        if(fault==0)a.scope.account="other";if(fault==1)a.run.value="other";if(fault==2)a.dispatch_epoch={2};if(fault==3)a.strategy.value="other";
        CHECK_EQ(gate.enqueue(a).first,Error::WrongIdentity);CHECK_EQ(gate.snapshot().registered,0U);
    }
    LocalDispatchGate gate(profile());auto a=auth();a.required_prefix.required_journal_seq={5};const auto result=gate.enqueue(a);CHECK(result.second);
    CHECK_EQ(gate.claim(*result.second,1),Error::Blocked);CHECK_EQ(gate.enqueue(a).first,Error::Duplicate);
    LocalDispatchGate expired(profile());const auto id=queued(expired);CHECK_EQ(expired.claim(id,100),Error::None);
    CHECK_EQ(expired.try_commit_call(id,101),Error::Blocked);CHECK_EQ(expired.state(id),std::optional{DispatchPermitState::AbortedBeforeCall});
}
TEST_CASE(GATE_repeat_commit_return_and_abort_cannot_authorize_second_call){
    LocalDispatchGate gate(profile());const auto id=queued(gate);CHECK_EQ(gate.claim(id,1),Error::None);
    CHECK_EQ(gate.try_commit_call(id,2),Error::None);CHECK_EQ(gate.try_commit_call(id,2),Error::Duplicate);
    CHECK_EQ(gate.abort_before_call(id),Error::Blocked);gate.close(LocalGateReason::Kill);CHECK(!gate.snapshot().quiesced);
    CHECK_EQ(gate.call_returned(id),Error::None);CHECK_EQ(gate.call_returned(id),Error::Duplicate);CHECK(gate.snapshot().quiesced);
    CHECK_EQ(gate.try_commit_call(id,3),Error::Duplicate);CHECK_EQ(gate.claim({8,1},3),Error::WrongIdentity);
}
TEST_CASE(GATE_real_thread_kill_between_claim_and_commit_prevents_fake_sdk_entry){
    LocalDispatchGate gate(profile());const auto id=queued(gate);std::barrier sync(2);
    auto adapter=std::async(std::launch::async,[&]{const auto c=gate.claim(id,1);sync.arrive_and_wait();sync.arrive_and_wait();const auto b=gate.try_commit_call(id,2);return std::pair{c,b};});
    sync.arrive_and_wait();gate.close(LocalGateReason::Kill);const auto before=gate.snapshot();sync.arrive_and_wait();const auto result=adapter.get();
    CHECK_EQ(result.first,Error::None);CHECK_EQ(result.second,Error::Halted);CHECK(!before.quiesced);CHECK(gate.snapshot().quiesced);
}
TEST_CASE(GATE_real_thread_committed_before_kill_stays_inflight_until_return){
    LocalDispatchGate gate(profile());const auto id=queued(gate);std::barrier sync(2);
    auto adapter=std::async(std::launch::async,[&]{const auto c=gate.claim(id,1),b=gate.try_commit_call(id,2);sync.arrive_and_wait();sync.arrive_and_wait();
        const bool entered=b==Error::None;const auto r=entered?gate.call_returned(id):Error::Blocked;return std::tuple{c,b,entered,r};});
    sync.arrive_and_wait();gate.close(LocalGateReason::Kill);const auto before=gate.snapshot();sync.arrive_and_wait();const auto result=adapter.get();
    CHECK_EQ(std::get<0>(result),Error::None);CHECK_EQ(std::get<1>(result),Error::None);CHECK(std::get<2>(result));CHECK_EQ(std::get<3>(result),Error::None);
    CHECK(!before.quiesced);CHECK_EQ(before.committed,1U);CHECK(gate.snapshot().quiesced);
}
TEST_CASE(GATE_capacity_and_counter_exhaustion_stop_without_reusing_permit_slots){
    auto c=profile();c.capacity=1;c.epoch={std::numeric_limits<std::uint64_t>::max()};LocalDispatchGate gate(c);
    auto a=auth();a.dispatch_epoch=c.epoch;CHECK_EQ(gate.enqueue(a).first,Error::None);a.authorization.serial=2;a.command.serial=3;
    CHECK_EQ(gate.enqueue(a).first,Error::Capacity);CHECK(!gate.snapshot().open);CHECK_EQ(gate.snapshot().epoch,c.epoch);
    CHECK((gate.snapshot().reasons&static_cast<std::uint32_t>(LocalGateReason::CounterExhausted))!=0);
}
TEST_CASE(GATE_market_suspension_blocks_claims_and_old_permissions_stay_invalid_after_requalification){
    LocalDispatchGate gate(profile());const auto old=queued(gate);gate.qualify_market(false);CHECK(!gate.snapshot().market_ready);CHECK(gate.snapshot().open);
    CHECK_EQ(gate.claim(old,1),Error::Blocked);gate.qualify_market(true);CHECK(gate.snapshot().market_ready);CHECK_EQ(gate.claim(old,2),Error::WrongIdentity);
    CHECK_EQ(gate.abort_before_call(old),Error::None);auto next=auth();next.authorization.serial=2;next.command.serial=3;next.order.serial=4;next.reservation.serial=5;next.dispatch_epoch=gate.snapshot().epoch;
    const auto accepted=gate.enqueue(next);CHECK(accepted.second);CHECK_EQ(gate.claim(*accepted.second,3),Error::None);
    gate.qualify_market(false);CHECK_EQ(gate.try_commit_call(*accepted.second,4),Error::Blocked);
    gate.close(LocalGateReason::Kill);gate.qualify_market(true);CHECK(!gate.snapshot().market_ready);CHECK(!gate.snapshot().open);CHECK(gate.snapshot().quiesced);
}
