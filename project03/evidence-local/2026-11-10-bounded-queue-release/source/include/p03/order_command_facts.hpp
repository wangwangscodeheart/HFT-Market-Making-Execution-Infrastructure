#pragma once
#include "p03/dispatch_facts.hpp"

namespace p03 {
// Raw vendor request reference is scoped to an observed source session/profile.
// No CTP integer encoding, OrderRef or OrderSysID conversion is assumed here.
struct VendorRequestIdentity {
    SourceSessionIdentity source_session;
    std::string profile_version,raw_request_id;
    [[nodiscard]] bool valid() const noexcept {
        return source_session.valid() && !profile_version.empty() && profile_version.size()<=128 &&
            !raw_request_id.empty() && raw_request_id.size()<=128;
    }
    bool operator==(const VendorRequestIdentity&) const = default;
};
struct NewOrderCommandFacts {
    DispatchAuthorizationFact authorization;
    OrderIntent intent;
    std::optional<ClientOrderId> client_id;
    std::optional<VendorRequestIdentity> vendor_request;
    StrategyId strategy{"fixture.default.strategy"};
    [[nodiscard]] Error validate_shape() const noexcept {
        const auto error=authorization.validate_shape();if(error!=Error::None)return error;
        const auto& scope=authorization.scope;
        if(!strategy.valid())return Error::InvalidInput;
        if(strategy!=authorization.strategy)return Error::WrongIdentity;
        if(intent.account!=scope.account || intent.instrument!=scope.contract.instrument || intent.day!=scope.day)
            return Error::WrongIdentity;
        if((intent.side!=Side::Buy && intent.side!=Side::Sell) ||
           (intent.offset!=Offset::Open && intent.offset!=Offset::CloseToday && intent.offset!=Offset::CloseYesterday) ||
           intent.price<=0 || intent.price>max_input_ticks || intent.quantity<=0 || intent.quantity>max_input_lots ||
           (client_id && !client_id->valid()) || (vendor_request && !vendor_request->valid()))return Error::InvalidInput;
        return Error::None;
    }
};
// M1A diagnostic value description, NOT a controlled send command or permission.
// CoreLoop does not mint this from a fabricated Journal identity or release it
// to an executor. That controlled path requires actual later-stage evidence.
} // namespace p03
