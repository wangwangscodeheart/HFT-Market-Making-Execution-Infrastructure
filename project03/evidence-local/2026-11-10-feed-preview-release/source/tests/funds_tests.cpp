#include "fixture.hpp"
#include <algorithm>
using namespace fixture;
namespace {
FundsLedger funds(Money cap=1000) { return {cap,0,"fixture.account",day,1}; }
FundsObservation observation(Money available,std::vector<CoverageProof> coverage={},std::uint64_t snapshot=1) {
    for(auto& p:coverage)
        if(p.status==CoverageStatus::Proven && p.external_deduction_id.empty())
            p.external_deduction_id="fixture.deduction."+std::to_string(p.resource.serial);
    return {"fixture.account",day,1,snapshot,available,EvidenceOrigin::LocalFixture,std::move(coverage),observation_metadata()};
}
}
TEST_CASE(P04_full_and_partial_proven_coverage_subtract_only_difference) {
    const ReservationId resource{7,2}; auto f=funds(); f.replace_resources({{resource,100,0,0}});
    CHECK_EQ(f.observe(observation(200,{{resource,CoverageStatus::Proven,100}})),Error::None);
    CHECK_EQ(f.view().local_required,100); CHECK_EQ(f.view().uncovered,0); CHECK_EQ(f.view().external_budget,200);
    CHECK_EQ(f.observe(observation(200,{{resource,CoverageStatus::Proven,60}},2)),Error::None);
    CHECK_EQ(f.view().uncovered,40); CHECK_EQ(f.view().external_budget,160);
    CHECK_EQ(f.view().proven_cover,60);
}
TEST_CASE(P04_local_cap_is_independent_of_external_available) {
    auto f=funds(99); f.replace_resources({{{7,2},100,0,0}});
    CHECK_EQ(f.observe(observation(1'000'000)),Error::None);
    CHECK(!f.view().local_ok); CHECK(f.view().external_ok); CHECK(!f.view().sufficient());
}
TEST_CASE(P04_duplicate_or_excess_coverage_rejected_atomically) {
    const ReservationId id{7,2}; auto f=funds(); f.replace_resources({{id,100,0,0}});
    CHECK_EQ(f.observe(observation(200,{{id,CoverageStatus::Proven,101}})),Error::InvalidInput);
    CHECK_EQ(f.observe(observation(200,{{id,CoverageStatus::Proven,60},{id,CoverageStatus::Proven,40}})),Error::InvalidInput);
    CHECK(!f.observation()); CHECK_EQ(f.view().proven_cover,0);
}
TEST_CASE(P04_core_insufficient_external_budget_rejects_atomic_reservation) {
    Fixture f; CHECK_EQ(f.apply(ObserveFunds{observation(200)}).error,Error::None);
    CHECK_EQ(f.apply(SubmitOrder{intent(),session()}).error,Error::InsufficientFunds);
    CHECK(f.core.state().orders.empty()); CHECK_EQ(f.core.state().funds.view().external_budget,200);
}
TEST_CASE(P05_unknown_and_excluded_coverage_are_zero) {
    const ReservationId id{7,2};
    for(const auto status:{CoverageStatus::Unknown,CoverageStatus::Excluded}) {
        auto f=funds(); f.replace_resources({{id,100,0,5}});
        CHECK_EQ(f.observe(observation(200,{{id,status,100}})),Error::None);
        CHECK_EQ(f.view().proven_cover,0); CHECK_EQ(f.view().uncovered,105); CHECK_EQ(f.view().external_budget,95);
    }
}
TEST_CASE(P05_untrusted_and_wrong_anchor_cannot_activate_coverage) {
    auto f=funds(); auto o=observation(200); o.origin=EvidenceOrigin::ExternalUnconfigured;
    CHECK_EQ(f.observe(o),Error::Blocked); o.origin=EvidenceOrigin::Unverified;
    CHECK_EQ(f.observe(o),Error::UntrustedEvidence); o.origin=EvidenceOrigin::LocalFixture; o.position_epoch=2;
    CHECK_EQ(f.observe(o),Error::WrongIdentity); CHECK(!f.observation());
}
TEST_CASE(P06_transfer_keeps_one_resource_and_no_fabricated_money) {
    Fixture f; const auto id=f.submit(); const auto resource=f.order(id).reservation();
    CHECK_EQ(f.apply(ObserveFunds{observation(1000,{{resource,CoverageStatus::Proven,300}})}).error,Error::None);
    const auto before=f.core.state().funds.view(); CHECK_EQ(before.external_budget,994);
    CHECK_EQ(f.apply(trade(id,"transfer")).error,Error::None);
    const auto& r=f.core.state().funds.resources().at(resource);
    CHECK_EQ(r.order_margin,200); CHECK_EQ(r.position_margin,100); CHECK_EQ(r.charges,6);
    CHECK_EQ(f.core.state().funds.resources().size(),2U);
    const auto after=f.core.state().funds.view(); CHECK_EQ(after.proven_cover,300);
    CHECK_EQ(after.external_budget,before.external_budget); CHECK_EQ(after.local_required,before.local_required);
}
TEST_CASE(P06_post_anchor_fees_are_not_covered_by_margin_proof) {
    const ReservationId id{7,2}; auto f=funds(); f.replace_resources({{id,0,100,5}});
    CHECK_EQ(f.observe(observation(10,{{id,CoverageStatus::Proven,100}})),Error::None);
    CHECK_EQ(f.view().external_budget,5);
    f.replace_resources({{id,0,100,8}}); CHECK_EQ(f.view().external_budget,2);
    CHECK_EQ(f.observe(observation(10,{{id,CoverageStatus::Proven,108}},2)),Error::InvalidInput);
    CHECK_EQ(f.view().external_budget,2);
}
TEST_CASE(P06_cancel_release_does_not_credit_external_available) {
    const ReservationId id{7,2}; auto f=funds(); f.replace_resources({{id,100,0,2}});
    CHECK_EQ(f.observe(observation(200,{{id,CoverageStatus::Proven,100}})),Error::None);
    f.replace_resources({{id,0,0,0}}); CHECK_EQ(f.view().external_budget,200);
    CHECK_EQ(f.observation()->external_available,200); CHECK_EQ(f.view().proven_cover,0);
}
TEST_CASE(FUNDS_same_snapshot_changed_payload_conflict_preserves_baseline) {
    auto f=funds(); const auto o=observation(200); CHECK_EQ(f.observe(o),Error::None);
    CHECK_EQ(f.observe(o),Error::Duplicate); auto changed=o; changed.external_available=300;
    CHECK_EQ(f.observe(changed),Error::EvidenceConflict); CHECK_EQ(f.observation()->external_available,200);
    changed.snapshot_id=0; CHECK_EQ(f.observe(changed),Error::InvalidInput);
}
TEST_CASE(FUNDS_duplicate_resource_transfer_cannot_double_proof) {
    const ReservationId id{7,2}; auto f=funds(); f.replace_resources({{id,100,0,0}});
    CHECK_THROWS((f.replace_resources({{id,50,0,0},{id,0,50,0}})),std::invalid_argument);
    CHECK_EQ(f.resources().size(),1U); CHECK_EQ(f.resources().at(id).order_margin,100);
}
TEST_CASE(FUNDS_invalid_coverage_status_cannot_activate_money) {
    const ReservationId id{7,2}; auto f=funds(); f.replace_resources({{id,100,0,0}});
    CHECK_EQ(f.observe(observation(200,{{id,static_cast<CoverageStatus>(99),100}})),Error::InvalidInput);
    CHECK(!f.observation()); CHECK_EQ(f.view().proven_cover,0);
}
TEST_CASE(P04_one_external_deduction_cannot_cover_two_resources) {
    const ReservationId a{7,2},b{7,3}; auto f=funds(); f.replace_resources({{a,100,0,0},{b,100,0,0}});
    CHECK_EQ(f.observe(observation(200,{{a,CoverageStatus::Proven,100,"same-deduction"},
        {b,CoverageStatus::Proven,100,"same-deduction"}})),Error::InvalidInput);
    CHECK(!f.observation()); CHECK_EQ(f.view().proven_cover,0);
}
TEST_CASE(P05_wrong_currency_and_unbounded_query_window_rejected) {
    auto f=funds(); auto o=observation(200); o.metadata.currency="UNKNOWN";
    CHECK_EQ(f.observe(o),Error::InvalidInput); o=observation(200);
    o.metadata.request_begin=5; o.metadata.response_end=4;
    CHECK_EQ(f.observe(o),Error::InvalidInput); CHECK(!f.observation());
}
TEST_CASE(P04_P06_coverage_lookup_matches_nested_reference_after_resource_changes){
    constexpr Money cap=50000,buffer=7;
    FundsLedger f(cap,buffer,"fixture.account",day,1);std::vector<FundResource> resources;std::vector<CoverageProof> proofs;
    for(std::uint64_t i=0;i<64;++i){
        const ReservationId id{7,i+1};resources.push_back({id,60,40,static_cast<Money>(i+1)});
        proofs.push_back({id,i%3==0?CoverageStatus::Proven:i%3==1?CoverageStatus::Unknown:CoverageStatus::Excluded,60,{}});
    }
    std::reverse(proofs.begin(),proofs.end());f.replace_resources(resources);
    CHECK_EQ(f.observe(observation(5000,proofs)),Error::None);
    auto compare=[&](){
        FundsView expected;
        for(const auto& [id,r]:f.resources()){
            expected.local_required=checked_add(expected.local_required,r.total());
            for(const auto& p:f.observation()->coverage)if(p.resource==id && p.status==CoverageStatus::Proven)
                expected.proven_cover=checked_add(expected.proven_cover,std::min(p.amount,r.margin()));
        }
        expected.uncovered=expected.local_required-expected.proven_cover;expected.local_ok=checked_add(expected.local_required,buffer)<=cap;
        expected.has_external=true;expected.external_budget=checked_add(checked_add(5000,-expected.uncovered),-buffer);expected.external_ok=expected.external_budget>=0;
        const auto actual=f.view();CHECK_EQ(actual.local_required,expected.local_required);CHECK_EQ(actual.proven_cover,expected.proven_cover);
        CHECK_EQ(actual.uncovered,expected.uncovered);CHECK_EQ(actual.external_budget,expected.external_budget);
        CHECK_EQ(actual.local_ok,expected.local_ok);CHECK_EQ(actual.external_ok,expected.external_ok);CHECK_EQ(actual.sufficient(),expected.sufficient());
    };
    compare();
    for(auto& r:resources){r.order_margin=0;r.position_margin=20;r.charges+=9;}f.replace_resources(resources);compare();
    resources.erase(resources.begin(),resources.begin()+32);f.replace_resources(resources);compare();
    f.replace_resources({});compare();CHECK_EQ(f.view().proven_cover,0);CHECK_EQ(f.view().external_budget,4993);
}
TEST_CASE(P04_coverage_replacement_failure_preserves_resources_and_view){
    auto f=funds();const ReservationId id{7,2};f.replace_resources({{id,100,0,5}});
    CHECK_EQ(f.observe(observation(200,{{id,CoverageStatus::Proven,60}})),Error::None);
    const auto resources=f.resources();const auto observed=f.observation();const auto before=f.view();
    CHECK_THROWS((f.replace_resources({{id,100,0,5},{id,20,0,1}})),std::invalid_argument);
    CHECK_THROWS((f.replace_resources({{id,std::numeric_limits<Money>::max(),1,0}})),std::overflow_error);
    CHECK_EQ(f.resources(),resources);CHECK_EQ(f.observation(),observed);
    CHECK_EQ(f.view().proven_cover,before.proven_cover);CHECK_EQ(f.view().local_required,before.local_required);CHECK_EQ(f.view().external_budget,before.external_budget);
}
