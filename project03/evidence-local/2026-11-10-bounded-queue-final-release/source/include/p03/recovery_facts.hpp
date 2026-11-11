#pragma once
#include "p03/types.hpp"
#include <set>
#include <vector>

namespace p03 {
// Value contracts only; no query execution, coverage proof, certificate or reset.
template<class Tag> struct LifecycleGeneration {
    std::uint64_t value{};
    auto operator<=>(const LifecycleGeneration&) const = default;
};
struct DispatchEpochTag;struct RecoveryEpochTag;struct LossGenerationTag;struct GateLatchTag;
using DispatchEpoch=LifecycleGeneration<DispatchEpochTag>;
using RecoveryEpoch=LifecycleGeneration<RecoveryEpochTag>;
using LossGeneration=LifecycleGeneration<LossGenerationTag>;
using GateLatchVersion=LifecycleGeneration<GateLatchTag>;
struct QueryTag;
using QueryId=Id<QueryTag>;
struct RunId {
    std::string value;
    [[nodiscard]] bool valid() const noexcept { return !value.empty() && value.size()<=128; }
    bool operator==(const RunId&) const = default;
};
struct SourceSessionIdentity {
    std::string source,session;
    std::uint64_t epoch{};
    [[nodiscard]] bool valid() const noexcept {
        return !source.empty() && source.size()<=128 && !session.empty() && session.size()<=128 && epoch!=0;
    }
    bool operator==(const SourceSessionIdentity&) const = default;
};
struct QueryCoverageIdentity {
    RunId run;
    std::string account;
    QueryId query;
    RecoveryEpoch recovery_epoch;
    LossGeneration loss_generation;
    SourceSessionIdentity source_session;
    [[nodiscard]] bool valid() const noexcept {
        return run.valid() && !account.empty() && account.size()<=128 && query.valid() &&
            recovery_epoch.value!=0 && source_session.valid();
    }
    bool operator==(const QueryCoverageIdentity&) const = default;
};
enum class ContractCoverage { ExplicitContracts,WholeAccount };
enum class QueryEvidenceKind { Orders,Trades,Positions,Funds };
enum class OrderCoverage { ActiveOnly,ActiveAndApplicableTerminal };
enum class CoverageBoundaryMethod { Unknown,SimRevisionWithLosslessTail,VerifiedStreamWatermark,ExternalQuiescenceAndFullHistory };
enum class QueryEndStatus { Incomplete,Complete,Failed };
struct QueryCoverageScope {
    ContractCoverage contracts{ContractCoverage::ExplicitContracts};
    std::vector<InstrumentKey> instruments;
    TradingDay first_day,last_day;
    QueryEvidenceKind kind{QueryEvidenceKind::Orders};
    std::optional<OrderCoverage> orders;
    std::string trade_identity_scope;
    [[nodiscard]] Error validate_shape() const {
        if(!first_day.valid() || !last_day.valid() || first_day>last_day)return Error::InvalidInput;
        if(contracts!=ContractCoverage::ExplicitContracts && contracts!=ContractCoverage::WholeAccount)return Error::InvalidInput;
        if(instruments.size()>256 || (contracts==ContractCoverage::ExplicitContracts && instruments.empty()) ||
           (contracts==ContractCoverage::WholeAccount && !instruments.empty()))return Error::InvalidInput;
        std::set<InstrumentKey> seen;
        for(const auto& key:instruments){
            if(key.exchange.empty() || key.exchange.size()>128 || key.instrument.empty() || key.instrument.size()>128 ||
               !seen.insert(key).second)return Error::InvalidInput;
        }
        if(kind<QueryEvidenceKind::Orders || kind>QueryEvidenceKind::Funds)return Error::InvalidInput;
        if(kind==QueryEvidenceKind::Orders){
            if(!orders || (*orders!=OrderCoverage::ActiveOnly && *orders!=OrderCoverage::ActiveAndApplicableTerminal))return Error::InvalidInput;
        }else if(orders)return Error::InvalidInput;
        if(trade_identity_scope.size()>128 || (kind==QueryEvidenceKind::Trades && trade_identity_scope.empty()))return Error::InvalidInput;
        return Error::None;
    }
};
struct QueryCoverageFact {
    QueryCoverageIdentity identity;
    QueryCoverageScope scope;
    std::string profile_version;
    CoverageBoundaryMethod boundary{CoverageBoundaryMethod::Unknown};
    QueryEndStatus end{QueryEndStatus::Incomplete};
    std::optional<std::uint64_t> expected_pages;
    std::uint64_t received_pages{};
    std::vector<std::string> missing_items,raw_evidence_refs;
    [[nodiscard]] Error validate_shape() const {
        if(!identity.valid() || profile_version.empty() || profile_version.size()>128)return Error::InvalidInput;
        const auto error=scope.validate_shape();if(error!=Error::None)return error;
        if(boundary<CoverageBoundaryMethod::Unknown || boundary>CoverageBoundaryMethod::ExternalQuiescenceAndFullHistory ||
           end<QueryEndStatus::Incomplete || end>QueryEndStatus::Failed)return Error::InvalidInput;
        if(received_pages>4096 || (expected_pages && (*expected_pages==0 || *expected_pages>4096 || received_pages>*expected_pages)))return Error::InvalidInput;
        if(end==QueryEndStatus::Complete && expected_pages && received_pages!=*expected_pages)return Error::InvalidInput;
        for(const auto* list:{&missing_items,&raw_evidence_refs}){
            if(list->size()>256)return Error::InvalidInput;
            for(const auto& value:*list)if(value.empty() || value.size()>128)return Error::InvalidInput;
        }
        return Error::None; // Well formed does NOT mean complete coverage or recovery eligibility.
    }
};
[[nodiscard]] inline Error validate_coverage_correlation(const QueryCoverageFact& fact,const QueryCoverageIdentity& expected) {
    const auto shape=fact.validate_shape();if(shape!=Error::None)return shape;
    if(!expected.valid())return Error::InvalidInput;
    return fact.identity==expected ? Error::None : Error::WrongIdentity;
}
} // namespace p03
