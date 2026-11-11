#include "fixture.hpp"
#include "p03/quote_strategy.hpp"
#include "../examples/local_sim_scenario.hpp"
using namespace fixture;
namespace {
MarketStateConfig market_config(){const auto c=config();return {{c.anchor.account,c.contract.key(),day,1},c.contract,EvidenceOrigin::LocalFixture,"fixture.book",1,1,2,10};}
MarketSnapshot snapshot(const MarketStateConfig& c,std::uint64_t sequence=1,LogicalTime at=1,std::uint64_t generation=1){
    MarketSnapshot b;b.scope=c.scope;b.origin=c.origin;b.source=c.source;b.source_epoch=c.source_epoch;
    b.subscription_generation=generation;b.receive_sequence=sequence;b.received_at=at;b.valid_depth=2;b.valid_fields=market_required_fields;
    b.bids[0]={498,10};b.bids[1]={496,20};b.asks[0]={502,10};b.asks[1]={504,20};return b;
}
QuoteStrategyConfig strategy_config(const CoreLoop& core){return {core.scope(),core.config().strategy,EvidenceOrigin::LocalFixture,"fixture.book",1,10,1,10};}
}
TEST_CASE(A02_MARKET_profile_requires_explicit_scope_depth_generation_and_age){
    for(int fault=0;fault<6;++fault){auto c=market_config();
        if(fault==0)c.required_depth=0;if(fault==1)c.required_depth=6;if(fault==2)c.initial_generation=0;
        if(fault==3)c.source_epoch=0;if(fault==4)c.max_age=0;if(fault==5)c.scope.contract.instrument="other";
        CHECK_THROWS(MarketState(c),std::invalid_argument);
    }
}
TEST_CASE(A02_MARKET_waiting_has_no_quote_then_full_snapshot_preserves_available_depth){
    const auto c=market_config();MarketState state(c);CHECK_EQ(state.health(0).status,FeedStatus::AwaitingData);CHECK(!state.top(0));
    auto b=snapshot(c);b.source_time=123;b.bids[4]={999999,0};
    CHECK_EQ(state.update(b,1),Error::None);CHECK_EQ(state.health(1).status,FeedStatus::Valid);
    CHECK_EQ(state.snapshot()->valid_depth,2U);CHECK_EQ(state.snapshot()->source_time,std::optional<std::uint64_t>{123});
    CHECK_EQ(state.top(1)->bid,498);CHECK_EQ(state.top(1)->subscription_generation,1U);
}
TEST_CASE(A02_MARKET_missing_fields_or_required_depth_invalidates_previous_quote){
    for(const int fault:{0,1,2}){const auto c=market_config();MarketState state(c);CHECK_EQ(state.update(snapshot(c),1),Error::None);
        auto bad=snapshot(c,2,2);if(fault==0)bad.valid_fields=7;if(fault==1)bad.valid_fields=31;if(fault==2)bad.valid_depth=1;
        CHECK_EQ(state.update(bad,2),Error::InvalidInput);CHECK_EQ(state.health(2).status,FeedStatus::Invalid);CHECK(!state.top(2));
        CHECK(state.snapshot().has_value());
    }
}
TEST_CASE(A02_MARKET_rejects_invalid_ticks_crosses_quantities_and_depth_order){
    for(int fault=0;fault<7;++fault){auto c=market_config();c.contract.tick_step=2;MarketState state(c);auto b=snapshot(c);
        if(fault==0)b.bids[0].price=499;if(fault==1)b.asks[0].price=498;if(fault==2)b.bids[0].quantity=0;
        if(fault==3)b.asks[0].quantity=max_input_lots+1;if(fault==4)b.bids[1].price=498;
        if(fault==5)b.asks[1].price=500;if(fault==6)b.valid_depth=6;
        CHECK_EQ(state.update(b,1),Error::InvalidInput);CHECK(!state.top(1));CHECK_EQ(state.health(1).status,FeedStatus::Invalid);
    }
}
TEST_CASE(A02_MARKET_expiry_boundary_and_duplicate_cannot_refresh_receive_age){
    const auto c=market_config();MarketState state(c);const auto b=snapshot(c);CHECK_EQ(state.update(b,1),Error::None);
    CHECK(state.top(11));CHECK_EQ(state.health(12).status,FeedStatus::Stale);CHECK(!state.top(12));
    CHECK_EQ(state.tick(12),Error::None);CHECK_EQ(state.update(b,12),Error::Duplicate);CHECK_EQ(state.health(12).status,FeedStatus::Stale);
    CHECK_EQ(state.update(snapshot(c,2,12),12),Error::None);CHECK(state.top(12));
}
TEST_CASE(A02_MARKET_reconnect_needs_new_generation_and_fresh_snapshot){
    const auto c=market_config();MarketState state(c);CHECK_EQ(state.update(snapshot(c,100),1),Error::None);
    CHECK_EQ(state.disconnect(2),Error::None);CHECK_EQ(state.health(2).status,FeedStatus::Recovering);CHECK(!state.top(2));
    CHECK_EQ(state.update(snapshot(c,101,3),3),Error::Blocked);CHECK_EQ(state.begin_subscription(1,3),Error::WrongIdentity);
    CHECK_EQ(state.begin_subscription(2,3),Error::None);CHECK(!state.top(3));
    CHECK_EQ(state.update(snapshot(c,102,4),4),Error::WrongIdentity);CHECK_EQ(state.health(4).status,FeedStatus::Recovering);
    CHECK_EQ(state.update(snapshot(c,1,4,2),4),Error::None);CHECK_EQ(state.top(4)->subscription_generation,2U);
    CHECK_EQ(state.update(snapshot(c,103,5),5),Error::WrongIdentity);CHECK(state.top(5));
}
TEST_CASE(A02_MARKET_local_counter_gaps_do_not_claim_exchange_packet_loss){
    const auto c=market_config();MarketState state(c);CHECK_EQ(state.update(snapshot(c),1),Error::None);
    CHECK_EQ(state.update(snapshot(c,100,2),2),Error::None);CHECK_EQ(state.health(2).status,FeedStatus::Valid);
    CHECK_EQ(state.local_loss(3),Error::None);CHECK_EQ(state.health(3).reason,FeedReason::LocalLoss);
    CHECK_EQ(state.update(snapshot(c,101,3),3),Error::Blocked);CHECK(!state.top(3));
    CHECK_EQ(state.begin_subscription(2,4),Error::None);CHECK_EQ(state.update(snapshot(c,1,4,2),4),Error::None);
}
TEST_CASE(A02_MARKET_invalid_current_snapshot_fences_older_or_changed_same_sequence){
    const auto c=market_config();MarketState state(c);CHECK_EQ(state.update(snapshot(c),1),Error::None);
    auto bad=snapshot(c,5,2);bad.valid_fields=0;CHECK_EQ(state.update(bad,2),Error::InvalidInput);
    CHECK_EQ(state.update(snapshot(c,3,2),2),Error::Duplicate);CHECK(!state.top(2));
    CHECK_EQ(state.update(snapshot(c,5,2),2),Error::EvidenceConflict);CHECK(!state.top(2));
    CHECK_EQ(state.update(snapshot(c,6,3),3),Error::None);CHECK(state.top(3));
}
TEST_CASE(A02_MARKET_foreign_profile_account_and_subscription_do_not_poison_current_book){
    for(int fault=0;fault<5;++fault){const auto c=market_config();MarketState state(c);CHECK_EQ(state.update(snapshot(c),1),Error::None);auto b=snapshot(c,2,2);
        if(fault==0)b.source="other";if(fault==1)b.source_epoch=2;if(fault==2)b.scope.account="other";
        if(fault==3)b.subscription_generation=2;if(fault==4)b.origin=EvidenceOrigin::ExternalUnconfigured;
        CHECK(state.update(b,2)!=Error::None);CHECK_EQ(state.health(2).status,FeedStatus::Valid);CHECK_EQ(state.top(2)->sequence,1U);
    }
}
TEST_CASE(A02_MARKET_current_source_day_change_latches_until_new_day_profile_is_built){
    const auto c=market_config();MarketState state(c);CHECK_EQ(state.update(snapshot(c),1),Error::None);
    auto next=snapshot(c,2,2);next.scope.day=next_day;CHECK_EQ(state.update(next,2),Error::WrongTradingDay);
    CHECK_EQ(state.health(2).reason,FeedReason::TradingDayMismatch);CHECK(!state.top(2));
    CHECK_EQ(state.update(snapshot(c,3,3),3),Error::WrongTradingDay);CHECK_EQ(state.begin_subscription(2,3),Error::WrongTradingDay);
    auto new_config=c;new_config.scope.day=next_day;new_config.contract.day=next_day;MarketState fresh(new_config);
    CHECK_EQ(fresh.update(snapshot(new_config,1,3),3),Error::None);CHECK(fresh.top(3));
}
TEST_CASE(A02_MARKET_future_or_regressing_receive_times_do_not_become_valid){
    const auto c=market_config();MarketState state(c);CHECK_EQ(state.update(snapshot(c,1,3),3),Error::None);
    CHECK_EQ(state.tick(2),Error::OutOfOrderTime);CHECK(!state.top(2));CHECK(state.top(3));
    CHECK_EQ(state.update(snapshot(c,2,5),4),Error::StaleMark);CHECK_EQ(state.health(4).reason,FeedReason::FutureTimestamp);
    CHECK_EQ(state.update(snapshot(c,3,2),4),Error::OutOfOrderTime);CHECK(!state.top(4));
    CHECK_EQ(state.update(snapshot(c,4,4),4),Error::None);CHECK(state.top(4));
}
TEST_CASE(A02_A03_MARKET_qualified_strategy_accepts_sequence_restart_only_in_new_subscription){
    Fixture f;const auto c=market_config();MarketState state(c);QuoteStrategy strategy(strategy_config(f.core));
    CHECK(strategy.evaluate(f.core,state,session(),0,1).withdraw);
    CHECK_EQ(state.update(snapshot(c,100),1),Error::None);const auto old=*state.top(1);
    CHECK_EQ(strategy.evaluate(f.core,state,session(),1,1).error,Error::None);
    CHECK_EQ(state.disconnect(2),Error::None);CHECK(strategy.evaluate(f.core,state,session(),2,2).withdraw);
    CHECK_EQ(state.begin_subscription(2,3),Error::None);CHECK_EQ(state.update(snapshot(c,1,3,2),3),Error::None);
    CHECK_EQ(strategy.evaluate(f.core,state,session(),3,2).error,Error::None);
    CHECK_EQ(strategy.evaluate(f.core,old,session(),3,3).error,Error::UntrustedEvidence);
    CHECK_EQ(strategy.evaluate(f.core,state,session(),14,3).error,Error::StaleMark);
}
TEST_CASE(A02_A03_MARKET_full_snapshot_to_strategy_planner_core_peer_and_invalid_feed_withdrawal){
    auto c=config();c.trade_evidence_mode=TradeEvidenceMode::Simulator;Fixture f(c);LocalSimVenue peer(example::simulator_config(c));
    const auto m=market_config();MarketState state(m);QuoteStrategy strategy(strategy_config(f.core));CHECK_EQ(state.update(snapshot(m),1),Error::None);
    const auto d=strategy.evaluate(f.core,state,session(),1,1);CHECK(d.target.has_value());const auto plan=plan_quotes(f.core,*d.target,1);CHECK_EQ(plan.intents.size(),2U);
    for(const auto& item:plan.intents){const auto& submit=std::get<SubmitOrder>(item);const auto accepted=f.apply(submit);CHECK(accepted.order);
        const auto reply=peer.receive({*accepted.order,submit.intent,c.strategy,submit.client_id});CHECK_EQ(reply.error,Error::None);
        for(const auto& report:reply.reports)CHECK_EQ(f.apply(report).error,Error::None);
    }
    auto bad=snapshot(m,2,5);bad.valid_depth=0;CHECK_EQ(state.update(bad,5),Error::InvalidInput);
    const auto stopped=strategy.evaluate(f.core,state,session(),5,2);CHECK(stopped.withdraw);CHECK(!stopped.target);
    const QuoteTarget empty{f.core.scope(),c.strategy,2,session(),{},{}};const auto withdraw=plan_quotes(f.core,empty,2);CHECK_EQ(withdraw.intents.size(),2U);
    for(const auto& item:withdraw.intents){const auto cancel=f.apply(std::get<RequestCancel>(item));CHECK(cancel.cancel);
        const auto reply=peer.cancel(std::get<RequestCancel>(item).order,*cancel.cancel);CHECK_EQ(reply.error,Error::None);
        for(const auto& report:reply.reports)CHECK_EQ(f.apply(report).error,Error::None);
    }
    for(const auto& [id,order]:f.core.state().orders){(void)id;CHECK_EQ(order.parts().held(),0);}
    CHECK_EQ(f.core.state().positions.booked(),peer.query({7,1}).snapshot->positions);
}
