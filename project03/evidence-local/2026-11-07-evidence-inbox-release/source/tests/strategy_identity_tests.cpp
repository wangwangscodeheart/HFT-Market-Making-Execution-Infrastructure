#include "fixture.hpp"
#include "p03/order_command_facts.hpp"
#include <type_traits>
using namespace fixture;
namespace {
SubmitOrder client_request(){return {intent(),session(),ClientOrderId{"fixture.client","request-1"},{}};}
}
TEST_CASE(STRATEGY_identity_is_distinct_and_invalid_configuration_cannot_boot) {
    static_assert(!std::is_convertible_v<StrategyId,ClientOrderId>);
    static_assert(!std::is_convertible_v<StrategyId,OrderId>);
    auto cfg=config();cfg.strategy.value.clear();CHECK_THROWS(CoreLoop{cfg},std::invalid_argument);
    cfg.strategy.value.assign(129,'x');CHECK_THROWS(CoreLoop{cfg},std::invalid_argument);
}
TEST_CASE(STRATEGY_omitted_owner_uses_configuration_not_client_producer) {
    auto cfg=config();cfg.strategy={"fixture.strategy.alpha"};Fixture f(cfg);const auto req=client_request();
    const auto accepted=f.apply(req);CHECK_EQ(accepted.error,Error::None);CHECK(accepted.order.has_value());
    CHECK_EQ(f.order(*accepted.order).strategy(),cfg.strategy);
    CHECK(f.order(*accepted.order).strategy().value!=req.client_id->producer);
    auto retry=req;retry.strategy=cfg.strategy;
    const auto duplicate=f.apply(retry);CHECK_EQ(duplicate.error,Error::Duplicate);CHECK_EQ(duplicate.order,accepted.order);
}
TEST_CASE(STRATEGY_foreign_owner_cannot_alias_or_modify_existing_client_request) {
    auto cfg=config();cfg.strategy={"fixture.strategy.alpha"};Fixture f(cfg);const auto req=client_request();
    const auto accepted=f.apply(req);CHECK(accepted.order.has_value());
    const auto resources=f.core.state().funds.resources();auto foreign=req;foreign.strategy=StrategyId{"fixture.strategy.beta"};
    const auto rejected=f.apply(foreign);CHECK_EQ(rejected.error,Error::WrongIdentity);
    CHECK_EQ(rejected.core_sequence,0U);CHECK(!rejected.order.has_value());
    CHECK_EQ(f.core.state().orders.size(),1U);CHECK_EQ(f.core.state().client_orders.size(),1U);
    CHECK_EQ(f.core.state().funds.resources(),resources);CHECK_EQ(f.order(*accepted.order).strategy(),cfg.strategy);
}
TEST_CASE(STRATEGY_client_index_key_qualifies_identical_client_ids_by_owner) {
    const ClientOrderId client{"fixture.client","same"};
    std::map<ClientRequestKey,int> index;
    index.emplace(ClientRequestKey{{"strategy.a"},client},1);
    index.emplace(ClientRequestKey{{"strategy.b"},client},2);
    CHECK_EQ(index.size(),2U);
    CHECK_EQ(index.at({{"strategy.a"},client}),1);CHECK_EQ(index.at({{"strategy.b"},client}),2);
    auto a=config(),b=config();a.strategy={"strategy.a"};b.strategy={"strategy.b"};
    CoreLoop ca(a),cb(b);CHECK(ca.canonical_state()!=cb.canonical_state());
}
TEST_CASE(STRATEGY_failed_commit_rolls_back_owned_client_index) {
    auto cfg=config();cfg.strategy={"fixture.strategy.alpha"};Fixture f(cfg);const auto req=client_request();
    f.core.fail_next_commit_for_test();CHECK_EQ(f.apply(req).error,Error::InjectedFailure);
    CHECK(f.core.state().client_orders.empty());CHECK(f.core.state().orders.empty());
    const auto accepted=f.apply(req);CHECK_EQ(accepted.error,Error::None);CHECK(accepted.order.has_value());
    CHECK_EQ(f.core.state().client_orders.at({cfg.strategy,*req.client_id}),*accepted.order);
}
TEST_CASE(STRATEGY_order_ownership_does_not_split_account_position_or_risk) {
    auto cfg=config({2,0,0,0});cfg.strategy={"fixture.strategy.alpha"};Fixture f(cfg);
    const auto buy=f.submit();const auto sell=f.submit(intent(Side::Sell,Offset::Open,1,501));
    CHECK_EQ(f.order(buy).strategy(),cfg.strategy);CHECK_EQ(f.order(sell).strategy(),cfg.strategy);
    CHECK_EQ(f.core.risk_view().min_net,1);CHECK_EQ(f.core.risk_view().max_net,5);
    CHECK_EQ(f.core.state().positions.booked(),(std::array<Lots,4>{2,0,0,0}));
}
TEST_CASE(STRATEGY_trade_transport_source_cannot_reassign_order_ownership) {
    auto cfg=config();cfg.strategy={"fixture.strategy.alpha"};Fixture f(cfg);const auto id=f.submit();
    auto report=trade(id,"trade-1");report.channel=Channel::Query;report.stamp.source="fixture.strategy.beta.query";
    CHECK_EQ(f.apply(report).error,Error::None);CHECK_EQ(f.order(id).strategy(),cfg.strategy);
    CHECK_EQ(f.order(id).booked(),1);CHECK_EQ(f.core.state().positions.booked()[0],1);
}
TEST_CASE(STRATEGY_command_description_requires_same_owner_as_authorization) {
    NewOrderCommandFacts facts{{{7,1},{7,2},{7,3},{7,4},{"fixture.run"},
        {"fixture.account",{"SHFE","rb_FIXTURE"},day,1},{1},{{{"fixture.journal"},{1}},{4}},100},intent(),{},{}};
    CHECK_EQ(facts.validate_shape(),Error::None);
    facts.strategy={"fixture.other.strategy"};CHECK_EQ(facts.validate_shape(),Error::WrongIdentity);
    facts.authorization.strategy=facts.strategy;CHECK_EQ(facts.validate_shape(),Error::None);
    facts.strategy.value.clear();CHECK_EQ(facts.validate_shape(),Error::InvalidInput);
}
