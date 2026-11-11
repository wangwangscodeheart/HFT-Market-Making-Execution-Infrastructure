#include "fixture.hpp"
#include "../examples/local_sim_scenario.hpp"
#include "p03/local_query_review.hpp"
using namespace fixture;
namespace {
SimConfig query_config(){return example::simulator_config(config());}
SimQuerySnapshot filled_query(){
    LocalSimVenue peer(query_config());CHECK_EQ(peer.receive({{7,1},intent(),StrategyId{"fixture.default.strategy"},{}}).error,Error::None);
    CHECK_EQ(peer.execute_fill({7,1},"query-t1",1,499).error,Error::None);
    return *peer.query({7,1}).snapshot;
}
}
TEST_CASE(QUERY_complete_snapshot_has_explicit_counts_and_independent_positions){
    const auto q=filled_query();CHECK(q.complete);CHECK_EQ(q.expected_orders,1U);CHECK_EQ(q.expected_trades,1U);
    CHECK_EQ(review_local_query(q,query_config()).status,LocalQueryStatus::Complete);
}
TEST_CASE(QUERY_missing_end_or_rows_cannot_be_treated_as_complete){
    auto q=filled_query();q.complete=false;CHECK_EQ(review_local_query(q,query_config()).status,LocalQueryStatus::Incomplete);
    q=filled_query();q.trades.clear();CHECK_EQ(review_local_query(q,query_config()).status,LocalQueryStatus::Incomplete);
    q.expected_trades=0;CHECK_EQ(review_local_query(q,query_config()).status,LocalQueryStatus::Incomplete);
    q=filled_query();q.orders.clear();CHECK_EQ(review_local_query(q,query_config()).status,LocalQueryStatus::Incomplete);
}
TEST_CASE(QUERY_duplicate_trade_and_foreign_scope_fail_validation){
    auto q=filled_query();q.trades.push_back(q.trades[0]);q.expected_trades=2;
    CHECK_EQ(review_local_query(q,query_config()).status,LocalQueryStatus::Invalid);
    q=filled_query();q.account="foreign";CHECK_EQ(review_local_query(q,query_config()).error,Error::WrongIdentity);
    q=filled_query();q.trades[0].trade.key.scope="p03.fixture.day.v1";
    CHECK_EQ(review_local_query(q,query_config()).status,LocalQueryStatus::Invalid);
}
TEST_CASE(QUERY_wrong_epoch_push_channel_and_future_revision_are_invalid){
    auto q=filled_query();++q.model_epoch;CHECK_EQ(review_local_query(q,query_config()).status,LocalQueryStatus::Invalid);
    q=filled_query();q.trades[0].channel=Channel::Push;CHECK_EQ(review_local_query(q,query_config()).status,LocalQueryStatus::Invalid);
    q=filled_query();q.trades[0].stamp.sequence=q.revision+1;
    CHECK_EQ(review_local_query(q,query_config()).status,LocalQueryStatus::Invalid);
}
TEST_CASE(QUERY_position_frozen_and_order_fill_disagreements_remain_unaccepted){
    auto q=filled_query();q.positions[0]=2;CHECK_EQ(review_local_query(q,query_config()).status,LocalQueryStatus::Invalid);
    q=filled_query();q.frozen[0]=1;CHECK_EQ(review_local_query(q,query_config()).status,LocalQueryStatus::Invalid);
    q=filled_query();q.orders[0].filled=2;CHECK_EQ(review_local_query(q,query_config()).status,LocalQueryStatus::Incomplete);
}
TEST_CASE(QUERY_review_never_changes_core_or_peer_state){
    Fixture f;LocalSimVenue peer(query_config());const auto before=f.core.canonical_state();
    const auto q=*peer.query({7,1}).snapshot;
    CHECK_EQ(review_local_query(q,query_config()).status,LocalQueryStatus::Complete);
    CHECK_EQ(f.core.canonical_state(),before);CHECK_EQ(peer.query({7,2}).snapshot->revision,0U);
}
TEST_CASE(QUERY_failed_response_missing_snapshot_and_wrong_request_cannot_be_accepted){
    SimQueryResult result{Error::None,filled_query()};
    CHECK_EQ(review_local_query(result,{7,1},query_config()).status,LocalQueryStatus::Complete);
    result.error=Error::InjectedFailure;
    CHECK_EQ(review_local_query(result,{7,1},query_config()).error,Error::InjectedFailure);
    result.error=Error::None;CHECK_EQ(review_local_query(result,{7,2},query_config()).error,Error::WrongIdentity);
    result.snapshot.reset();CHECK_EQ(review_local_query(result,{7,1},query_config()).status,LocalQueryStatus::Incomplete);
    CHECK_EQ(review_local_query(result,{},query_config()).error,Error::InvalidInput);
}
TEST_CASE(QUERY_core_correlation_rejects_unknown_orders_and_changed_intent_before_booking){
    Fixture f;SimQueryResult response{Error::None,filled_query()};
    CHECK_EQ(review_local_query_for_core(response,{7,1},query_config(),f.core).error,Error::UnknownOrder);
    const auto id=f.submit();CHECK_EQ(id,(OrderId{7,1}));
    CHECK_EQ(review_local_query_for_core(response,{7,1},query_config(),f.core).status,LocalQueryStatus::Complete);
    response.snapshot->orders[0].request.strategy={"other.strategy"};
    CHECK_EQ(review_local_query_for_core(response,{7,1},query_config(),f.core).error,Error::EvidenceConflict);
    response={Error::None,filled_query()};response.snapshot->orders[0].request.intent.quantity=4;
    CHECK_EQ(review_local_query_for_core(response,{7,1},query_config(),f.core).error,Error::EvidenceConflict);
    CHECK_EQ(f.core.state().positions.booked()[0],0);
}
TEST_CASE(QUERY_core_correlation_rejects_missing_known_orders_and_already_booked_trade){
    Fixture f;const auto id=f.submit();LocalSimVenue peer(query_config());
    CHECK_EQ(peer.receive({id,intent(),f.core.config().strategy,{}}).error,Error::None);
    CHECK_EQ(f.apply(trade(id,"not-in-peer-query",1)).error,Error::None);
    CHECK_EQ(review_local_query_for_core(peer.query({7,1}),{7,1},query_config(),f.core).status,LocalQueryStatus::Incomplete);
    Fixture unfilled;unfilled.submit();LocalSimVenue empty(query_config());
    CHECK_EQ(review_local_query_for_core(empty.query({7,1}),{7,1},query_config(),unfilled.core).status,LocalQueryStatus::Incomplete);
}
TEST_CASE(QUERY_available_budget_must_equal_reconstructed_resources_not_just_be_in_range){
    auto q=filled_query();CHECK_EQ(review_local_query(q,query_config()).status,LocalQueryStatus::Complete);
    ++q.available_budget;CHECK_EQ(review_local_query(q,query_config()).status,LocalQueryStatus::Invalid);
    q=filled_query();--q.available_budget;CHECK_EQ(review_local_query(q,query_config()).status,LocalQueryStatus::Invalid);
}
TEST_CASE(QUERY_margin_and_fee_components_cannot_be_moved_or_forged_between_buckets){
    for(int field=0;field<4;++field){
        auto q=filled_query();auto& o=q.orders[0];
        if(field==0)++o.reserved_margin;if(field==1)++o.position_margin;if(field==2)++o.reserved_fees;if(field==3)++o.paid_fees;
        CHECK_EQ(review_local_query(q,query_config()).status,LocalQueryStatus::Invalid);
    }
    auto q=filled_query();q.orders[0].paid_fees=-1;CHECK_EQ(review_local_query(q,query_config()).status,LocalQueryStatus::Invalid);
}
TEST_CASE(QUERY_budget_review_accepts_working_partial_canceled_and_rejected_peer_orders){
    const auto c=query_config();LocalSimVenue peer(c);const SimOrderRequest request{{7,1},intent(),StrategyId{"fixture.default.strategy"},{}};
    CHECK_EQ(peer.receive(request).error,Error::None);
    CHECK_EQ(review_local_query(*peer.query({7,1}).snapshot,c).status,LocalQueryStatus::Complete);
    CHECK_EQ(peer.execute_fill({7,1},"budget-fill",1,499).error,Error::None);
    CHECK_EQ(review_local_query(*peer.query({7,2}).snapshot,c).status,LocalQueryStatus::Complete);
    CHECK_EQ(peer.cancel({7,1},{7,1}).error,Error::None);
    CHECK_EQ(review_local_query(*peer.query({7,3}).snapshot,c).status,LocalQueryStatus::Complete);
    auto rejected=request;rejected.order={7,2};rejected.intent.price=2000;
    CHECK_EQ(peer.receive(rejected).error,Error::PriceRule);
    auto q=*peer.query({7,4}).snapshot;CHECK_EQ(review_local_query(q,c).status,LocalQueryStatus::Complete);
    q.orders[1].reserved_fees=1;CHECK_EQ(review_local_query(q,c).status,LocalQueryStatus::Invalid);
}
TEST_CASE(QUERY_close_budget_retains_seed_margin_and_charges_only_explicit_model_fees){
    auto c=query_config();c.initial_positions={2,1,0,0};LocalSimVenue peer(c);
    SimOrderRequest request{{7,1},intent(Side::Sell,Offset::CloseToday,1,501),StrategyId{"fixture.default.strategy"},{}};
    CHECK_EQ(peer.receive(request).error,Error::None);CHECK_EQ(peer.execute_fill({7,1},"close-budget",1,501).error,Error::None);
    const auto q=*peer.query({7,1}).snapshot;CHECK_EQ(q.available_budget,999698);
    CHECK_EQ(review_local_query(q,c).status,LocalQueryStatus::Complete);
    auto incorrect=q;incorrect.available_budget+=100;CHECK_EQ(review_local_query(incorrect,c).status,LocalQueryStatus::Invalid);
}
TEST_CASE(QUERY_invalid_budget_configuration_and_accepted_order_rules_fail_closed){
    const auto q=filled_query();auto c=query_config();c.fee_per_lot=-1;
    CHECK_EQ(review_local_query(q,c).status,LocalQueryStatus::Invalid);
    c=query_config();c.rules.tick_step=0;CHECK_EQ(review_local_query(q,c).status,LocalQueryStatus::Invalid);
    auto wrong=q;wrong.orders[0].request.intent.price=2000;CHECK_EQ(review_local_query(wrong,query_config()).status,LocalQueryStatus::Invalid);
}
