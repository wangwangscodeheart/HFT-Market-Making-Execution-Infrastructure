#pragma once
#include "p03/dispatch_facts.hpp"
#include <array>
#include <mutex>
namespace p03 {
enum class LocalGateReason : std::uint32_t {Kill=1,ReportLoss=2,WriterFailure=4,HeartbeatExpired=8,Capacity=16,CounterExhausted=32,EvidenceFailure=64};
struct LocalGateConfig {
    std::uint64_t instance{};RunId run;BusinessScope scope;StrategyId strategy;
    DispatchEpoch epoch{1};RequiredJournalPrefix memory_prefix;
    std::size_t capacity{64};
};
struct LocalGateSnapshot {
    bool open{},quiesced{};DispatchEpoch epoch;GateLatchVersion latch;LossGeneration loss;
    std::uint32_t reasons{};std::size_t registered{},claimed{},committed{};
    bool market_ready{true};
};
// Local-memory experiment only: memory_prefix is NOT a Writer/fsync attestation.
// No SDK entry, reservation release, recovery certificate or reopen operation.
class LocalDispatchGate {
public:
    explicit LocalDispatchGate(LocalGateConfig config):config_(std::move(config)),epoch_(config_.epoch){
        if(config_.instance==0 || !config_.run.valid() || !config_.strategy.valid() || epoch_.value==0 ||
           config_.scope.account.empty() || config_.scope.account.size()>128 || !config_.scope.day.valid() || config_.scope.position_epoch==0 ||
           config_.scope.contract.exchange.empty() || config_.scope.contract.instrument.empty() ||
           config_.scope.contract.exchange.size()>128 || config_.scope.contract.instrument.size()>128 ||
           config_.capacity==0 || config_.capacity>slots_.size() || config_.memory_prefix.validate_shape()!=Error::None)
            throw std::invalid_argument("explicit local memory gate profile required");
    }
    std::pair<Error,std::optional<DispatchPermitId>> enqueue(DispatchAuthorizationFact fact){
        const auto shape=fact.validate_shape();if(shape!=Error::None)return {shape,{}};
        std::lock_guard lock(mutex_);
        if(!open_)return {Error::Halted,{}};
        if(!market_ready_)return {Error::Blocked,{}};
        if(fact.authorization.instance!=config_.instance || fact.scope!=config_.scope || fact.run!=config_.run ||
           fact.strategy!=config_.strategy || fact.dispatch_epoch!=epoch_ || fact.required_prefix.writer!=config_.memory_prefix.writer)
            return {Error::WrongIdentity,{}};
        for(std::size_t i=0;i<count_;++i)if(slots_[i]->authorization.command==fact.command || slots_[i]->authorization.authorization==fact.authorization)
            return {Error::Duplicate,{}};
        if(count_==config_.capacity){close_locked(LocalGateReason::Capacity);return {Error::Capacity,{}};}
        const DispatchPermitId id{config_.instance,static_cast<std::uint64_t>(count_+1)};
        slots_[count_].emplace(DispatchPermitFact{id,std::move(fact),DispatchPermitState::Queued});++count_;
        return {Error::None,id};
    }
    Error claim(DispatchPermitId id,LogicalTime now){
        std::lock_guard lock(mutex_);auto* p=find(id);if(!p)return Error::WrongIdentity;
        if(p->state!=DispatchPermitState::Queued)return Error::Duplicate;
        const auto eligible=eligibility(*p,now);if(eligible!=Error::None)return eligible;
        p->state=DispatchPermitState::Claimed;return Error::None;
    }
    Error try_commit_call(DispatchPermitId id,LogicalTime now){
        std::lock_guard lock(mutex_);auto* p=find(id);if(!p)return Error::WrongIdentity;
        if(p->state!=DispatchPermitState::Claimed)return Error::Duplicate;
        const auto eligible=eligibility(*p,now);
        if(eligible!=Error::None){p->state=DispatchPermitState::AbortedBeforeCall;return eligible;}
        p->state=DispatchPermitState::CallCommitted;return Error::None;
    }
    Error call_returned(DispatchPermitId id){
        std::lock_guard lock(mutex_);auto* p=find(id);if(!p)return Error::WrongIdentity;
        if(p->state!=DispatchPermitState::CallCommitted)return Error::Duplicate;
        p->state=DispatchPermitState::CallReturned;return Error::None;
    }
    Error abort_before_call(DispatchPermitId id){
        std::lock_guard lock(mutex_);auto* p=find(id);if(!p)return Error::WrongIdentity;
        if(p->state!=DispatchPermitState::Queued && p->state!=DispatchPermitState::Claimed)return Error::Blocked;
        p->state=DispatchPermitState::AbortedBeforeCall;return Error::None;
    }
    void close(LocalGateReason reason){std::lock_guard lock(mutex_);close_locked(reason);}
    void qualify_market(bool ready){
        std::lock_guard lock(mutex_);
        if(ready){if(open_)market_ready_=true;return;}
        if(!market_ready_)return;market_ready_=false;
        if(epoch_.value==std::numeric_limits<std::uint64_t>::max())close_locked(LocalGateReason::CounterExhausted);
        else ++epoch_.value; // Old queued/claimed permissions remain invalid after feed recovery.
    }
    [[nodiscard]] LocalGateSnapshot snapshot() const {
        std::lock_guard lock(mutex_);LocalGateSnapshot s{open_,false,epoch_,latch_,loss_,reasons_,count_,0,0};
        for(std::size_t i=0;i<count_;++i){s.claimed+=slots_[i]->state==DispatchPermitState::Claimed;s.committed+=slots_[i]->state==DispatchPermitState::CallCommitted;}
        s.market_ready=market_ready_;s.quiesced=!open_ && s.claimed==0 && s.committed==0;return s;
    }
    [[nodiscard]] std::optional<DispatchPermitState> state(DispatchPermitId id) const {
        std::lock_guard lock(mutex_);if(id.instance!=config_.instance || id.serial==0 || id.serial>count_)return {};
        return slots_[static_cast<std::size_t>(id.serial-1)]->state;
    }
private:
    DispatchPermitFact* find(DispatchPermitId id){
        if(id.instance!=config_.instance || id.serial==0 || id.serial>count_)return nullptr;
        return &*slots_[static_cast<std::size_t>(id.serial-1)];
    }
    Error eligibility(const DispatchPermitFact& p,LogicalTime now) const {
        if(!open_)return Error::Halted;
        if(!market_ready_)return Error::Blocked;
        if(p.authorization.dispatch_epoch!=epoch_)return Error::WrongIdentity;
        if(now>p.authorization.expires_at)return Error::Blocked;
        if(p.authorization.required_prefix.required_journal_seq>config_.memory_prefix.required_journal_seq)return Error::Blocked;
        return Error::None;
    }
    void increment(std::uint64_t& value){if(value==std::numeric_limits<std::uint64_t>::max())reasons_|=static_cast<std::uint32_t>(LocalGateReason::CounterExhausted);else ++value;}
    void close_locked(LocalGateReason reason){
        open_=false;const auto bit=static_cast<std::uint32_t>(reason);
        if((reasons_&bit)==0){increment(epoch_.value);increment(latch_.value);}reasons_|=bit;
        if(reason==LocalGateReason::ReportLoss)increment(loss_.value);
    }
    const LocalGateConfig config_;mutable std::mutex mutex_;std::array<std::optional<DispatchPermitFact>,64> slots_{};
    std::size_t count_{};bool open_{true};DispatchEpoch epoch_;GateLatchVersion latch_{1};LossGeneration loss_{};std::uint32_t reasons_{};
    bool market_ready_{true};
};
} // namespace p03
