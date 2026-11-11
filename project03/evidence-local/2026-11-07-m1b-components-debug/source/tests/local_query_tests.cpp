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
