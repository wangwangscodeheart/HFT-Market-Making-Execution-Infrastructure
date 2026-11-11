#pragma once
#include "p03/journal_facts.hpp"
#include "p03/recovery_facts.hpp"

namespace p03 {
struct AuthorizationTag;struct PermitTag;
using DispatchAuthorizationId=Id<AuthorizationTag>;
using DispatchPermitId=Id<PermitTag>;
struct DispatchAuthorizationFact {
    DispatchAuthorizationId authorization;
    CommandId command;
    OrderId order;
    ReservationId reservation;
    RunId run;
    BusinessScope scope;
    DispatchEpoch dispatch_epoch;
    RequiredJournalPrefix required_prefix;
    LogicalTime expires_at{};
    StrategyId strategy{"fixture.default.strategy"};
    [[nodiscard]] Error validate_shape() const noexcept {
        if(!authorization.valid() || !command.valid() || !order.valid() || !reservation.valid() || !run.valid() ||
           dispatch_epoch.value==0 || expires_at==0 || !strategy.valid())return Error::InvalidInput;
        if(command.instance!=authorization.instance || order.instance!=authorization.instance ||
           reservation.instance!=authorization.instance)return Error::WrongIdentity;
        if(scope.account.empty() || scope.account.size()>128 || scope.contract.exchange.empty() ||
           scope.contract.exchange.size()>128 || scope.contract.instrument.empty() || scope.contract.instrument.size()>128 ||
           !scope.day.valid() || scope.position_epoch==0)return Error::InvalidInput;
        return required_prefix.validate_shape();
    }
};
enum class DispatchPermitState { Queued,Claimed,CallCommitted,CallReturned,AbortedBeforeCall };
struct DispatchPermitFact {
    DispatchPermitId permit;
    DispatchAuthorizationFact authorization;
    DispatchPermitState state{DispatchPermitState::Queued};
    [[nodiscard]] Error validate_shape() const noexcept {
        const auto error=authorization.validate_shape();if(error!=Error::None)return error;
        if(!permit.valid())return Error::InvalidInput;
        if(permit.instance!=authorization.authorization.instance)return Error::WrongIdentity;
        return state>=DispatchPermitState::Queued && state<=DispatchPermitState::AbortedBeforeCall ? Error::None : Error::InvalidInput;
    }
};
struct StopEvidenceFacts {
    std::optional<bool> kill_accepted,new_order_calls_quiesced,external_risk_resolved;
};
// These copyable facts are NOT authorization capabilities. There is no public
// claim/TryCommitCall/SDK path here; real checks and transitions belong to M1C/D.
} // namespace p03
