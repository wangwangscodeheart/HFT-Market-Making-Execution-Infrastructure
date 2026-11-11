#include "fixture.hpp"
#include "p03/local_runtime_runner.hpp"
#include "p03/local_query_review.hpp"
#include "../examples/quote_runner_scenario.hpp"
using namespace fixture;
namespace {
LocalRuntimeConfig quoted(){auto b=example::quote_runner_config();b.strategy.max_age=100;
    MarketStateConfig m{b.strategy.scope,b.core.contract,b.strategy.origin,b.strategy.source,b.strategy.market_epoch,1,2,100};
    return {b.core,{b.peer},128,16,RuntimeQuoteConfig{m,b.strategy}};
}
RuntimeMarketSnapshot book(const LocalRuntimeConfig& c,std::uint64_t seq,LogicalTime at,Ticks bid=499,Ticks ask=501,std::uint64_t generation=1){
    const auto& m=c.quote->market;MarketSnapshot b;b.scope=m.scope;b.origin=m.origin;b.source=m.source;b.source_epoch=m.source_epoch;
    b.subscription_generation=generation;b.receive_sequence=seq;b.received_at=at;b.valid_depth=2;b.valid_fields=market_required_fields;
    b.bids[0]={bid,10};b.bids[1]={bid-1,10};b.asks[0]={ask,10};b.asks[1]={ask+1,10};
    return {b,{c.core.contract.day,9*3600+60,c.core.contract.day}};
}
bool ready(LocalRuntimeRunner& run,std::size_t n){const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(3);
    while(std::chrono::steady_clock::now()<until){if(run.pending_outcomes()>=n)return true;std::this_thread::yield();}return false;}
void aligned(const LocalRuntimeConfig& c,const LocalRuntimeRunner& run){const auto& q=*run.peer_query();CHECK_EQ(compare_local_query_with_core(q,q.snapshot->query,c.adapter.peer,run.core()).status,LocalQueryStatus::Complete);}
}
TEST_CASE(RUNTIME_QUOTE_reprice_waits_for_both_async_cancel_terminals_before_replacement){
    const auto c=quoted();LocalRuntimeRunner run(c);CHECK_EQ(run.step(RuntimeStart{},0),Error::None);CHECK_EQ(run.step(book(c,1,1),1),Error::None);
    CHECK_EQ(run.core().state().orders.size(),2U);CHECK(ready(run,2));CHECK_EQ(run.step(RuntimePump{8},2),Error::None);
    CHECK_EQ(run.step(book(c,2,3,498,502),3),Error::None);CHECK_EQ(run.core().state().orders.size(),2U);CHECK_EQ(run.trace().back().plan->intents.size(),2U);
    for(const auto& intent:run.trace().back().plan->intents)CHECK(std::holds_alternative<RequestCancel>(intent));
    CHECK(ready(run,2));CHECK_EQ(run.step(RuntimePump{1},4),Error::Blocked);CHECK_EQ(run.core().state().orders.size(),2U);
    CHECK_EQ(run.step(RuntimePump{8},5),Error::None);CHECK_EQ(run.core().state().orders.size(),4U);
    for(std::uint64_t serial=1;serial<=2;++serial)CHECK_EQ(run.core().state().orders.at({7,serial}).parts().held(),0);
    CHECK(ready(run,2));CHECK_EQ(run.step(RuntimePump{8},6),Error::None);CHECK_EQ(run.step(KillRequested{},7),Error::None);
    CHECK_EQ(run.step(RuntimeFinish{},8),Error::None);CHECK_EQ(run.step(RuntimePump{8},9),Error::None);aligned(c,run);CHECK_EQ(run.core().state().mode,CoreMode::Killed);
}
TEST_CASE(RUNTIME_QUOTE_disconnect_requires_new_subscription_and_fresh_snapshot){
    const auto c=quoted();LocalRuntimeRunner run(c);run.step(RuntimeStart{},0);run.step(book(c,1,1),1);CHECK(ready(run,2));CHECK_EQ(run.step(RuntimePump{8},2),Error::None);
    CHECK_EQ(run.step(RuntimeFeedDisconnect{},3),Error::None);CHECK_EQ(run.trace().back().feed->status,FeedStatus::Recovering);
    CHECK(ready(run,2));CHECK_EQ(run.step(RuntimePump{8},4),Error::Blocked);CHECK_EQ(run.core().state().orders.size(),2U);
    CHECK_EQ(run.step(book(c,2,5),5),Error::Blocked);CHECK_EQ(run.step(RuntimeFeedSubscription{2},6),Error::None);CHECK_EQ(run.trace().back().feed->status,FeedStatus::Recovering);
    CHECK_EQ(run.step(book(c,1,7,499,501,2),7),Error::None);CHECK_EQ(run.core().state().orders.size(),4U);
    CHECK(ready(run,2));CHECK_EQ(run.step(RuntimePump{8},8),Error::None);CHECK_EQ(run.step(RuntimeFinish{},9),Error::None);aligned(c,run);
}
TEST_CASE(RUNTIME_QUOTE_timer_expires_original_snapshot_and_raw_manual_inputs_cannot_bypass_mode){
    const auto c=quoted();LocalRuntimeRunner run(c);CHECK_EQ(run.step(ReferenceMark{500},0),Error::Blocked);CHECK(run.trace().empty());
    run.step(RuntimeStart{},0);const auto first=book(c,1,1);run.step(first,1);CHECK(ready(run,2));CHECK_EQ(run.step(RuntimePump{8},2),Error::None);
    CHECK_EQ(run.step(RuntimeQuoteTimer{first.session},102),Error::StaleMark);CHECK_EQ(run.trace().back().feed->status,FeedStatus::Stale);
    CHECK(ready(run,2));CHECK_EQ(run.step(RuntimePump{8},103),Error::StaleMark);for(const auto& [id,o]:run.core().state().orders){(void)id;CHECK_EQ(o.parts().held(),0);}
    CHECK_EQ(run.step(RuntimeFinish{},104),Error::None);aligned(c,run);
    auto invalid=c;invalid.quote->strategy.source="foreign";CHECK_THROWS(LocalRuntimeRunner(invalid),std::invalid_argument);
}
TEST_CASE(RUNTIME_QUOTE_same_snapshot_refreshes_inventory_after_fill_without_requoting_risk_side){
    auto c=quoted();c.quote->strategy.quantity=3;c.quote->strategy.inventory_limit=3;LocalRuntimeRunner run(c);
    run.step(RuntimeStart{},0);run.step(book(c,1,1),1);CHECK(ready(run,2));CHECK_EQ(run.step(RuntimePump{8},2),Error::None);
    CHECK_EQ(run.step(RuntimeFill{{7,1},"inventory-limit",3,499},3),Error::None);CHECK(ready(run,1));CHECK_EQ(run.step(RuntimePump{8},4),Error::None);
    CHECK_EQ(run.core().state().positions.booked()[0],3);CHECK(!run.trace().back().decision->target->buy);CHECK_EQ(run.trace().back().feed->last_receive_sequence,1U);
    CHECK_EQ(run.core().state().orders.size(),2U);CHECK(ready(run,1));CHECK_EQ(run.step(RuntimePump{8},5),Error::None);CHECK_EQ(run.core().state().orders.size(),3U);
    const auto& replacement=run.core().state().orders.at({7,3});CHECK_EQ(replacement.intent().side,Side::Sell);CHECK_EQ(replacement.intent().offset,Offset::CloseToday);CHECK_EQ(replacement.intent().quantity,3);
    CHECK(ready(run,1));CHECK_EQ(run.step(RuntimePump{8},6),Error::None);CHECK_EQ(run.step(RuntimeFinish{},7),Error::None);aligned(c,run);CHECK(run.core().invariant());
}
