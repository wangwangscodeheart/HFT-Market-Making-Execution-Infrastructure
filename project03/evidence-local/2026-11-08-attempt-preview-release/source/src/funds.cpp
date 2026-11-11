#include "p03/funds.hpp"
#include <algorithm>
#include <set>

namespace p03 {
FundsLedger::FundsLedger(Money cap,Money buffer,std::string account,TradingDay day,std::uint64_t epoch)
    : local_cap_(cap),safety_buffer_(buffer),account_(std::move(account)),day_(day),position_epoch_(epoch) {
    if(cap<0 || cap>max_money || buffer<0 || buffer>max_money || account_.empty() ||
       account_.size()>128 || !day.valid() || epoch==0) throw std::invalid_argument("funds configuration");
}
void FundsLedger::replace_resources(std::vector<FundResource> resources) {
    std::map<ReservationId,FundResource> candidate;
    for(const auto& r:resources) {
        if(!r.id.valid() || r.order_margin<0 || r.position_margin<0 || r.charges<0 ||
           !candidate.emplace(r.id,r).second)
            throw std::invalid_argument("invalid/duplicate fund resource");
        (void)r.total();
    }
    resources_.swap(candidate);
}
Error FundsLedger::observe(FundsObservation o) {
    const auto error=validate(o);
    if(error!=Error::None) return error;
    observation_=std::move(o);
    return Error::None;
}
Error FundsLedger::validate(const FundsObservation& o) const {
    if(o.origin!=EvidenceOrigin::LocalFixture)
        return o.origin==EvidenceOrigin::ExternalUnconfigured ? Error::Blocked : Error::UntrustedEvidence;
    if(o.account!=account_ || o.day!=day_ || o.position_epoch!=position_epoch_) return Error::WrongIdentity;
    const auto& m=o.metadata;
    if(m.source.empty() || m.source.size()>128 || !m.source.starts_with("fixture.") ||
       m.profile!="p03.fixture.v1" || m.scope.empty() || m.scope.size()>128 ||
       m.currency!="FIXTURE_UNIT" || m.request_begin>m.response_end) return Error::InvalidInput;
    if(o.snapshot_id==0 || o.external_available < -max_money || o.external_available>max_money ||
       o.coverage.size()>resources_.size()) return Error::InvalidInput;
    if(observation_) {
        if(o.snapshot_id<observation_->snapshot_id) return Error::WrongIdentity;
        if(o.snapshot_id==observation_->snapshot_id)
            return o==*observation_ ? Error::Duplicate : Error::EvidenceConflict;
    }
    std::set<ReservationId> seen;
    std::set<std::string> deduction_ids;
    for(const auto& p:o.coverage) {
        const auto it=resources_.find(p.resource);
        if(it==resources_.end() || p.amount<0 || p.amount>max_money || !seen.insert(p.resource).second)
            return Error::InvalidInput;
        if(p.status!=CoverageStatus::Proven && p.status!=CoverageStatus::Unknown && p.status!=CoverageStatus::Excluded)
            return Error::InvalidInput;
        if(p.external_deduction_id.size()>128) return Error::InvalidInput;
        if(p.status==CoverageStatus::Proven && (p.external_deduction_id.empty() ||
           !deduction_ids.insert(p.external_deduction_id).second)) return Error::InvalidInput;
        if(p.status==CoverageStatus::Proven && p.amount>it->second.margin()) return Error::InvalidInput;
    }
    return Error::None;
}
FundsView FundsLedger::view() const {
    FundsView v;
    for(const auto& [id,r]:resources_) {
        (void)id;
        v.local_required=checked_add(v.local_required,r.total());
    }
    if(observation_)for(const auto& proof:observation_->coverage){
        if(proof.status!=CoverageStatus::Proven)continue;
        const auto found=resources_.find(proof.resource);
        if(found!=resources_.end())v.proven_cover=checked_add(v.proven_cover,std::min(proof.amount,found->second.margin()));
    }
    v.uncovered=v.local_required-v.proven_cover;
    v.local_ok=checked_add(v.local_required,safety_buffer_)<=local_cap_;
    v.has_external=observation_.has_value();
    if(observation_) v.external_budget=checked_add(checked_add(observation_->external_available,-v.uncovered),-safety_buffer_);
    v.external_ok=!v.has_external || v.external_budget>=0;
    return v;
}
} // namespace p03
