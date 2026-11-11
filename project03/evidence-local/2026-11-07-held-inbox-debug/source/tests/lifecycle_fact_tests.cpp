#include "test.hpp"
#include "p03/dispatch_facts.hpp"
#include <type_traits>
using namespace p03;
namespace {
DispatchAuthorizationFact authorization(){return {{7,1},{7,2},{7,3},{7,4},{"fixture.run"},
    {"fixture.account",{"SHFE","rb_FIXTURE"},{20261106},1},{1},{{{"fixture.journal"},{1}},{4}},100};}
QueryCoverageFact coverage(){return {{{"fixture.run"},"fixture.account",{7,1},{1},{0},{"fixture.query","fixture.session",1}},
    {ContractCoverage::ExplicitContracts,{{"SHFE","rb_FIXTURE"}},{20261105},{20261106},QueryEvidenceKind::Orders,OrderCoverage::ActiveAndApplicableTerminal,{}},
    "fixture.profile.v1",CoverageBoundaryMethod::Unknown,QueryEndStatus::Incomplete,{},0,{"boundary-unverified"},{}};}
}
TEST_CASE(DISPATCH_FACTS_authorization_domains_are_distinct_and_same_instance) {
    static_assert(!std::is_convertible_v<DispatchAuthorizationId,CommandId>);
    static_assert(!std::is_convertible_v<DispatchPermitId,OrderId>);
    static_assert(!std::is_same_v<DispatchEpoch,RecoveryEpoch>);
    static_assert(!std::is_same_v<LossGeneration,GateLatchVersion>);
    CHECK_EQ(authorization().validate_shape(),Error::None);
    for(int variant=0;variant<4;++variant){auto a=authorization();
        if(variant==0)a.command.instance=8;if(variant==1)a.order.instance=8;
        if(variant==2)a.reservation.instance=8;if(variant==3)a.authorization.instance=8;
        CHECK_EQ(a.validate_shape(),Error::WrongIdentity);
    }
}
TEST_CASE(DISPATCH_FACTS_missing_scope_prefix_epoch_or_expiry_cannot_form_valid_facts) {
    for(int variant=0;variant<8;++variant){auto a=authorization();
        if(variant==0)a.scope.account.clear();if(variant==1)a.scope.contract.instrument.clear();
        if(variant==2)a.scope.day={};if(variant==3)a.scope.position_epoch=0;
        if(variant==4)a.dispatch_epoch.value=0;if(variant==5)a.expires_at=0;
        if(variant==6)a.required_prefix.required_journal_seq.value=0;if(variant==7)a.run.value.clear();
        CHECK_EQ(a.validate_shape(),Error::InvalidInput);
    }
}
TEST_CASE(DISPATCH_FACTS_permit_names_all_five_states_without_running_a_gate) {
    for(const auto state:{DispatchPermitState::Queued,DispatchPermitState::Claimed,DispatchPermitState::CallCommitted,
        DispatchPermitState::CallReturned,DispatchPermitState::AbortedBeforeCall}){
        CHECK_EQ((DispatchPermitFact{{7,9},authorization(),state}.validate_shape()),Error::None);
    }
    CHECK_EQ((DispatchPermitFact{{8,9},authorization(),DispatchPermitState::Claimed}.validate_shape()),Error::WrongIdentity);
    CHECK_EQ((DispatchPermitFact{{7,9},authorization(),static_cast<DispatchPermitState>(99)}.validate_shape()),Error::InvalidInput);
    const StopEvidenceFacts stop;CHECK(!stop.kill_accepted.has_value());CHECK(!stop.new_order_calls_quiesced.has_value());CHECK(!stop.external_risk_resolved.has_value());
}
TEST_CASE(RECOVERY_FACTS_unknown_boundary_and_missing_items_are_not_hidden) {
    auto fact=coverage();CHECK_EQ(fact.validate_shape(),Error::None);
    fact.end=QueryEndStatus::Complete;fact.expected_pages=1;fact.received_pages=1;
    fact.raw_evidence_refs={"fixture.empty-query-page"};
    CHECK_EQ(fact.validate_shape(),Error::None);
    CHECK_EQ(fact.boundary,CoverageBoundaryMethod::Unknown);CHECK(!fact.missing_items.empty());
    CHECK_EQ(fact.scope.orders.value(),OrderCoverage::ActiveAndApplicableTerminal);
}
TEST_CASE(RECOVERY_FACTS_wrong_run_account_query_session_or_generation_cannot_correlate) {
    const auto fact=coverage();CHECK_EQ(validate_coverage_correlation(fact,fact.identity),Error::None);
    for(int variant=0;variant<7;++variant){auto expected=fact.identity;
        if(variant==0)expected.run.value="fixture.other.run";if(variant==1)expected.account="fixture.other.account";
        if(variant==2)expected.query.serial=2;if(variant==3)expected.recovery_epoch.value=2;
        if(variant==4)expected.loss_generation.value=1;if(variant==5)expected.source_session.session="fixture.other.session";
        if(variant==6)expected.source_session.epoch=2;
        CHECK_EQ(validate_coverage_correlation(fact,expected),Error::WrongIdentity);
    }
}
TEST_CASE(RECOVERY_FACTS_explicit_and_whole_account_scopes_are_not_interchangeable) {
    auto fact=coverage();fact.scope.instruments.clear();CHECK_EQ(fact.validate_shape(),Error::InvalidInput);
    fact.scope.contracts=ContractCoverage::WholeAccount;CHECK_EQ(fact.validate_shape(),Error::None);
    fact.scope.instruments={{"SHFE","rb_FIXTURE"}};CHECK_EQ(fact.validate_shape(),Error::InvalidInput);
    fact=coverage();fact.scope.instruments.push_back(fact.scope.instruments.front());CHECK_EQ(fact.validate_shape(),Error::InvalidInput);
    fact=coverage();fact.scope.instruments.push_back({"OTHER_VENUE","rb_FIXTURE"});CHECK_EQ(fact.validate_shape(),Error::None);
}
TEST_CASE(RECOVERY_FACTS_scope_kind_day_and_trade_identity_are_explicit) {
    auto fact=coverage();fact.scope.first_day={20261107};CHECK_EQ(fact.validate_shape(),Error::InvalidInput);
    fact=coverage();fact.scope.kind=QueryEvidenceKind::Trades;CHECK_EQ(fact.validate_shape(),Error::InvalidInput);
    fact.scope.orders.reset();CHECK_EQ(fact.validate_shape(),Error::InvalidInput);
    fact.scope.trade_identity_scope="fixture.trade.day.scope";CHECK_EQ(fact.validate_shape(),Error::None);
    fact=coverage();fact.scope.orders=OrderCoverage::ActiveOnly;CHECK_EQ(fact.validate_shape(),Error::None);
    CHECK_EQ(fact.scope.orders.value(),OrderCoverage::ActiveOnly); // Valid description, not sufficient recovery coverage.
}
TEST_CASE(RECOVERY_FACTS_page_mismatch_and_unbounded_metadata_are_rejected) {
    auto fact=coverage();fact.expected_pages=2;fact.received_pages=1;fact.end=QueryEndStatus::Complete;
    CHECK_EQ(fact.validate_shape(),Error::InvalidInput);
    fact.end=QueryEndStatus::Incomplete;CHECK_EQ(fact.validate_shape(),Error::None);
    fact.received_pages=3;CHECK_EQ(fact.validate_shape(),Error::InvalidInput);
    fact=coverage();fact.received_pages=4097;CHECK_EQ(fact.validate_shape(),Error::InvalidInput);
    fact=coverage();fact.missing_items.assign(257,"fixture.missing");CHECK_EQ(fact.validate_shape(),Error::InvalidInput);
    fact=coverage();fact.raw_evidence_refs={std::string(129,'x')};CHECK_EQ(fact.validate_shape(),Error::InvalidInput);
    fact=coverage();fact.identity.recovery_epoch.value=0;CHECK_EQ(fact.validate_shape(),Error::InvalidInput);
}
