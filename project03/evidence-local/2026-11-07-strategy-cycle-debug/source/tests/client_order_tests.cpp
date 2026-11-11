#include "fixture.hpp"
#include "../examples/demo_scenario.hpp"
#include "p03/order_command_facts.hpp"
#include <type_traits>
using namespace fixture;
namespace {
SubmitOrder request(std::string producer="fixture.client",std::string value="request-1"){
    return {intent(),session(),ClientOrderId{std::move(producer),std::move(value)}};
}
NewOrderCommandFacts command_facts(){
    const auto scenario=example::standard_filled_order();const auto& submit=std::get<SubmitOrder>(scenario.inputs[1]);
    return {{{7,1},{7,2},{7,1},{7,2},{"fixture.run"},
        {scenario.config.anchor.account,scenario.config.contract.key(),scenario.config.anchor.day,1},
        {1},{{{"fixture.journal"},{1}},{4}},100},submit.intent,submit.client_id,{}};
}
}
TEST_CASE(CLIENT_same_request_after_partial_fill_does_not_duplicate_risk_or_order) {
    Fixture f;const auto req=request();const auto first=f.apply(req);CHECK(first.order.has_value());
    CHECK_EQ(f.apply(trade(*first.order,"fill-1")).error,Error::None);
    const auto resources=f.core.state().funds.resources();auto retry=req;retry.session.second+=1;
    const auto duplicate=f.apply(retry);CHECK_EQ(duplicate.error,Error::Duplicate);CHECK(!duplicate.applied);
    CHECK_EQ(duplicate.order,first.order);CHECK_EQ(f.core.state().orders.size(),1U);
    CHECK_EQ(f.core.state().client_orders.size(),1U);CHECK_EQ(f.core.state().funds.resources(),resources);
    CHECK_EQ(f.order(*first.order).booked(),1);CHECK(!f.order(*first.order).external_identity().has_value());
}
TEST_CASE(CLIENT_same_key_changed_intent_is_rejected_without_poisoning_original) {
    for(int variant=0;variant<4;++variant){Fixture f;const auto req=request();const auto first=f.apply(req);
        CHECK(first.order.has_value());const auto resources=f.core.state().funds.resources();auto changed=req;
        if(variant==0)changed.intent.quantity=2;if(variant==1)changed.intent.price=498;
        if(variant==2)changed.intent.side=Side::Sell;if(variant==3)changed.intent.offset=Offset::CloseToday;
        const auto conflict=f.apply(changed);CHECK_EQ(conflict.error,Error::EvidenceConflict);CHECK(!conflict.applied);
        CHECK_EQ(conflict.order,first.order);CHECK_EQ(f.core.state().orders.size(),1U);
        CHECK_EQ(f.order(*first.order).intent(),req.intent);CHECK_EQ(f.core.state().funds.resources(),resources);
        CHECK_EQ(f.order(*first.order).consistency(),Consistency::InSync);CHECK_EQ(f.core.state().mode,CoreMode::Ready);
    }
}
TEST_CASE(CLIENT_producer_namespace_prevents_accidental_request_collision) {
    Fixture f;const auto a=f.apply(request("fixture.client.a","same"));const auto b=f.apply(request("fixture.client.b","same"));
    CHECK_EQ(a.error,Error::None);CHECK_EQ(b.error,Error::None);CHECK(a.order!=b.order);
    CHECK_EQ(f.core.state().orders.size(),2U);CHECK_EQ(f.core.state().client_orders.size(),2U);
}
TEST_CASE(CLIENT_invalid_identity_rejected_before_business_sequence) {
    for(int variant=0;variant<3;++variant){Fixture f;auto req=request();
        if(variant==0)req.client_id->value.clear();if(variant==1)req.client_id->producer.clear();
        if(variant==2)req.client_id->value.assign(129,'x');
        const auto result=f.apply(req);CHECK_EQ(result.error,Error::InvalidInput);CHECK_EQ(result.core_sequence,0U);
        CHECK(f.core.state().orders.empty());CHECK(f.core.state().client_orders.empty());
        CHECK_EQ(f.core.state().funds.view().local_required,0);
    }
}
TEST_CASE(CLIENT_failed_reservation_does_not_bind_request_to_nonexistent_order) {
    auto cfg=config();cfg.risk.local_funds=200;Fixture f(cfg);auto req=request();
    CHECK_EQ(f.apply(req).error,Error::InsufficientFunds);CHECK(f.core.state().client_orders.empty());
    req.intent.quantity=1;const auto accepted=f.apply(req);CHECK_EQ(accepted.error,Error::None);CHECK(accepted.order.has_value());
    CHECK_EQ(accepted.order->serial,1U);CHECK_EQ(f.core.state().client_orders.size(),1U);
}
TEST_CASE(CLIENT_commit_failure_rolls_back_index_order_and_resources_together) {
    Fixture f;const auto req=request();f.core.fail_next_commit_for_test();
    CHECK_EQ(f.apply(req).error,Error::InjectedFailure);CHECK(f.core.state().client_orders.empty());CHECK(f.core.state().orders.empty());
    CHECK_EQ(f.core.state().funds.view().local_required,0);
    const auto accepted=f.apply(req);CHECK_EQ(accepted.error,Error::None);CHECK(accepted.order.has_value());CHECK_EQ(accepted.order->serial,1U);
}
TEST_CASE(CLIENT_known_retry_while_killed_reports_original_without_new_risk) {
    Fixture f;const auto req=request();const auto first=f.apply(req);CHECK(first.order.has_value());
    CHECK_EQ(f.apply(KillRequested{}).error,Error::None);const auto resources=f.core.state().funds.resources();
    const auto duplicate=f.apply(req);CHECK_EQ(duplicate.error,Error::Duplicate);CHECK_EQ(duplicate.order,first.order);
    CHECK_EQ(f.core.state().mode,CoreMode::Killed);CHECK_EQ(f.core.state().funds.resources(),resources);
    CHECK_EQ(f.apply(request("fixture.client","new-request")).error,Error::Halted);
}
TEST_CASE(CLIENT_duplicate_at_order_capacity_does_not_need_a_second_slot) {
    auto cfg=config();cfg.capacity.orders=1;Fixture f(cfg);const auto req=request();
    CHECK_EQ(f.apply(req).error,Error::None);CHECK_EQ(f.apply(req).error,Error::Duplicate);
    CHECK_EQ(f.apply(request("fixture.client","new")).error,Error::Capacity);
    CHECK_EQ(f.core.state().client_orders.size(),1U);CHECK_EQ(f.core.state().orders.size(),1U);
}
TEST_CASE(CLIENT_retry_after_full_fill_does_not_open_a_new_position) {
    const auto scenario=example::standard_filled_order();CoreLoop core(scenario.config);
    LogicalTime time=0;
    for(const auto& input:scenario.inputs)CHECK_EQ(core.step(input,time++).error,Error::None);
    const auto resources=core.state().funds.resources();
    const auto retry=core.step(scenario.inputs[1],time);
    CHECK_EQ(retry.error,Error::Duplicate);CHECK_EQ(retry.order,std::optional<OrderId>{scenario.order});
    CHECK(!retry.applied);CHECK_EQ(core.state().orders.size(),1U);
    CHECK_EQ(core.state().positions.booked()[0],10);CHECK_EQ(core.state().funds.resources(),resources);
    CHECK_EQ(core.state().orders.at(scenario.order).execution(),ExecutionState::Filled);CHECK(core.invariant());
}
TEST_CASE(CLIENT_unknown_order_timeout_retry_never_resubmits_or_releases) {
    Fixture f;const auto req=request();const auto first=f.apply(req);CHECK(first.order.has_value());
    CHECK_EQ(f.apply(OrderTimeout{*first.order}).error,Error::None);
    const auto resources=f.core.state().funds.resources();const auto retry=f.apply(req);
    CHECK_EQ(retry.error,Error::Duplicate);CHECK_EQ(retry.order,first.order);CHECK(!retry.applied);
    CHECK_EQ(f.core.state().orders.size(),1U);CHECK_EQ(f.core.state().funds.resources(),resources);
    CHECK_EQ(f.order(*first.order).parts().held(),3);CHECK_EQ(f.order(*first.order).consistency(),Consistency::Unreconciled);
}
TEST_CASE(COMMAND_FACTS_intent_must_match_authorized_scope_and_preserve_vendor_unknown) {
    static_assert(!std::is_convertible_v<ClientOrderId,OrderId>);
    const auto base=command_facts();CHECK_EQ(base.validate_shape(),Error::None);CHECK(!base.vendor_request.has_value());
    for(int variant=0;variant<3;++variant){auto facts=base;
        if(variant==0)facts.intent.account="other";if(variant==1)facts.intent.instrument="other";if(variant==2)facts.intent.day=next_day;
        CHECK_EQ(facts.validate_shape(),Error::WrongIdentity);
    }
}
TEST_CASE(COMMAND_FACTS_vendor_reference_requires_session_and_profile_not_invented_CTP_mapping) {
    auto facts=command_facts();facts.vendor_request=VendorRequestIdentity{{"fixture.vendor","session",1},"fixture.vendor.v1","raw-42"};
    CHECK_EQ(facts.validate_shape(),Error::None);
    facts.vendor_request->source_session.epoch=0;CHECK_EQ(facts.validate_shape(),Error::InvalidInput);
    facts=command_facts();facts.vendor_request=VendorRequestIdentity{{"fixture.vendor","session",1},{},"raw-42"};
    CHECK_EQ(facts.validate_shape(),Error::InvalidInput);
}
