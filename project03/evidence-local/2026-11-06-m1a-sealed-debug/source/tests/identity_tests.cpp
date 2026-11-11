#include "fixture.hpp"
#include <map>
using namespace fixture;

TEST_CASE(IDENTITY_empty_account_states_have_distinct_diagnostic_summaries) {
    auto first=config(),second=config();
    second.anchor.account="fixture.other.account";
    CoreLoop a(first),b(second);
    CHECK(a.state().orders.empty());CHECK(b.state().orders.empty());
    CHECK_EQ(a.state().positions.booked(),b.state().positions.booked());
    CHECK(a.scope()!=b.scope());
    CHECK(a.canonical_state()!=b.canonical_state());
    CHECK(a.canonical_state().starts_with("m1a.v5|"));
    CHECK(a.invariant());CHECK(b.invariant());
}

TEST_CASE(IDENTITY_exchange_qualifies_contract_without_case_guessing) {
    std::map<InstrumentKey,int> catalog;
    catalog.emplace(InstrumentKey{"VENUE_A","same_symbol"},1);
    catalog.emplace(InstrumentKey{"VENUE_B","same_symbol"},2);
    catalog.emplace(InstrumentKey{"VENUE_A","SAME_SYMBOL"},3);
    CHECK_EQ(catalog.size(),3U);
    CHECK_EQ(catalog.at({"VENUE_A","same_symbol"}),1);
    CHECK_EQ(catalog.at({"VENUE_B","same_symbol"}),2);
    CHECK(!InstrumentKey{"VENUE_A","same_symbol"}.matches("VENUE_B","same_symbol"));
    CHECK_EQ(config().contract.key(),(InstrumentKey{"SHFE","rb_FIXTURE"}));
}

TEST_CASE(IDENTITY_distinct_core_scopes_do_not_share_orders_positions_or_dedup) {
    Fixture a;
    auto cfg=config();cfg.instance=8;cfg.anchor.account="fixture.account.b";
    cfg.contract.instrument="rb_FIXTURE_B";cfg.anchor.instrument=cfg.contract.instrument;
    Fixture b(cfg);
    CHECK_EQ(a.core.scope(),(BusinessScope{"fixture.account",{"SHFE","rb_FIXTURE"},day,1}));
    CHECK_EQ(b.core.scope(),(BusinessScope{"fixture.account.b",{"SHFE","rb_FIXTURE_B"},day,1}));
    const auto aid=a.submit();auto bi=intent();bi.account=cfg.anchor.account;bi.instrument=cfg.contract.instrument;
    const auto bid=b.submit(bi);
    CHECK_EQ(aid.serial,bid.serial);CHECK(aid!=bid);
    const auto at=trade(aid,"same.raw.trade.id");auto bt=trade(bid,"same.raw.trade.id",2);
    bt.trade.key.account=cfg.anchor.account;bt.trade.instrument=cfg.contract.instrument;
    CHECK_EQ(a.apply(at).error,Error::None);CHECK_EQ(b.apply(bt).error,Error::None);
    CHECK_EQ(a.core.state().positions.booked()[0],1);CHECK_EQ(b.core.state().positions.booked()[0],2);
    CHECK_EQ(a.order(aid).booked(),1);CHECK_EQ(b.order(bid).booked(),2);
    auto query=bt;query.channel=Channel::Query;query.stamp=stamp({},"fixture.query");
    CHECK_EQ(b.apply(query).error,Error::Duplicate);
    CHECK_EQ(a.core.state().executions.records().size(),1U);CHECK_EQ(b.core.state().executions.records().size(),1U);
    CHECK(!a.core.state().orders.contains(bid));CHECK(!b.core.state().orders.contains(aid));
}

TEST_CASE(IDENTITY_foreign_trade_never_books_or_releases_resources) {
    for(int variant=0;variant<5;++variant){
        Fixture f;const auto id=f.submit();auto t=trade(id,"foreign");
        if(variant==0)t.trade.key.account="fixture.foreign.account";
        if(variant==1)t.trade.key.exchange="OTHER_EXCHANGE";
        if(variant==2)t.trade.instrument="rb_OTHER";
        if(variant==3)t.trade.key.day=next_day;
        if(variant==4)t.trade.order={8,id.serial};
        const auto funds=f.core.state().funds.view().local_required;
        const auto result=f.apply(t);
        CHECK_EQ(result.error,variant==3?Error::WrongTradingDay:variant==4?Error::UnknownOrder:Error::WrongIdentity);
        CHECK(result.applied); // Quarantining evidence is a state change; booking is not.
        CHECK_EQ(f.core.state().mode,CoreMode::ConflictHalted);
        CHECK(f.core.state().executions.records().empty());
        CHECK_EQ(f.order(id).booked(),0);CHECK_EQ(f.order(id).parts().released,0);
        CHECK_EQ(f.core.state().positions.booked(),(std::array<Lots,4>{}));
        CHECK_EQ(f.core.state().funds.view().local_required,funds);
        CHECK_EQ(f.core.state().quarantine.size(),1U);CHECK(f.core.state().quarantine.back().trade.has_value());
    }
}

TEST_CASE(IDENTITY_foreign_intent_is_rejected_without_consuming_order_ID) {
    for(int variant=0;variant<3;++variant){
        Fixture f;auto i=intent();
        if(variant==0)i.account="fixture.foreign";
        if(variant==1)i.instrument="rb_OTHER";
        if(variant==2)i.day=next_day;
        const auto result=f.apply(SubmitOrder{i,session()});
        CHECK_EQ(result.error,variant==0?Error::WrongIdentity:variant==1?Error::NoContract:Error::WrongTradingDay);
        CHECK(!result.applied);CHECK(f.core.state().orders.empty());
        CHECK_EQ(f.core.state().funds.view().local_required,0);
        CHECK_EQ(f.submit().serial,1U);
    }
}

TEST_CASE(IDENTITY_external_observations_cannot_overwrite_scoped_position_book) {
    Fixture f(config({2,3,4,5}));const auto before=f.core.state().positions.booked();
    auto o=position_observation(1,{10,20,30,40});o.account="fixture.foreign";
    CHECK_EQ(f.apply(ObservePosition{o}).error,Error::WrongIdentity);
    o=position_observation(2,{10,20,30,40});o.instrument="rb_OTHER";
    CHECK_EQ(f.apply(ObservePosition{o}).error,Error::WrongIdentity);
    CHECK(f.core.state().external_positions.empty());CHECK_EQ(f.core.state().positions.booked(),before);
    o=position_observation(3,{10,20,30,40});o.day=next_day;
    CHECK_EQ(f.apply(ObservePosition{o}).error,Error::None);
    CHECK_EQ(f.core.state().external_positions.size(),1U);
    CHECK_EQ(f.core.state().positions.booked(),before);CHECK_EQ(f.core.scope().day,day);
}

TEST_CASE(IDENTITY_scope_boot_preserves_single_RB_contract_boundary) {
    for(int variant=0;variant<4;++variant){
        auto cfg=config();
        if(variant==0)cfg.anchor.instrument="rb_OTHER";
        if(variant==1)cfg.anchor.day=next_day;
        if(variant==2)cfg.contract.exchange="OTHER_EXCHANGE";
        if(variant==3)cfg.contract.product="OTHER_PRODUCT";
        CHECK_THROWS(CoreLoop{cfg},std::invalid_argument);
    }
}
