#pragma once
#include "test.hpp"
#include "p03/step_harness.hpp"

namespace fixture {
using namespace p03;
inline constexpr TradingDay day{20261105};
inline constexpr TradingDay next_day{20261106};
inline CoreConfig config(std::array<Lots,4> initial={}) {
    Contract contract{"rb_FIXTURE","RB","SHFE","fixture.contract.v1",day,100,1000,1,1,10,1,true};
    TradingCalendar calendar{"fixture.calendar.v1",{
        {{20261104},21*3600,23*3600,day},
        {day,9*3600,10*3600+15*60,day},
        {day,10*3600+30*60,11*3600+30*60,day},
        {day,13*3600+30*60,15*3600,day}}};
    PositionAnchor anchor{"fixture.account","rb_FIXTURE",day,1,"fixture.initial.no-trades-included",initial};
    RiskLimits risk; risk.max_mark_age=1'000'000;
    for(const auto q:initial) risk.anchor_margin=checked_add(risk.anchor_margin,checked_mul_nonnegative(q,100));
    return {7,1,std::move(contract),std::move(calendar),std::move(anchor),risk,{}};
}
inline SessionPoint session() { return {day,9*3600+60,day}; }
inline OrderIntent intent(Side side=Side::Buy,Offset offset=Offset::Open,Lots quantity=3,Ticks price=499) {
    return {"fixture.account","rb_FIXTURE",day,side,offset,price,quantity};
}
inline EvidenceStamp stamp(std::optional<std::uint64_t> seq={},std::string source="fixture.orders") {
    return {EvidenceOrigin::LocalFixture,std::move(source),1,seq,seq.has_value()};
}
inline EvidenceHeader header(OrderId id,std::optional<std::uint64_t> seq={}) {
    return {id,"fixture.account",day,stamp(seq)};
}
inline ObservationMetadata observation_metadata() {
    return {"fixture.query","p03.fixture.v1","fixture.account-instrument","FIXTURE_UNIT",0,1};
}
inline ExternalPositionObservation position_observation(std::uint64_t id=1,std::array<Lots,4> quantities={}) {
    return {day,id,quantities,"fixture.account","rb_FIXTURE",EvidenceOrigin::LocalFixture,
        observation_metadata(),"p03.fixture.four-buckets.v1"};
}
inline SettlementObservation settlement_observation(std::uint64_t id=1,Lots total=0) {
    return {day,id,total,"fixture.account",EvidenceOrigin::LocalFixture,observation_metadata()};
}
inline TradeReport trade(OrderId id,std::string tid,Lots q=1,Side side=Side::Buy,Offset offset=Offset::Open,Ticks price=499) {
    return {{{"fixture.account",day,"SHFE","p03.fixture.day.v1",std::move(tid)},id,"rb_FIXTURE",side,offset,q,price},
        stamp({},"fixture.trades"),Channel::Push};
}
struct Fixture {
    CoreLoop core;
    LogicalTime time{};
    explicit Fixture(CoreConfig c=config()) : core(std::move(c)) {
        CHECK_EQ(core.step(ReferenceMark{500},time).error,Error::None); CHECK(core.invariant());
    }
    StepResult apply(BusinessEvent e) { auto r=core.step(e,++time); CHECK(core.invariant()); return r; }
    OrderId submit(OrderIntent i=intent()) {
        const auto r=apply(SubmitOrder{std::move(i),session()}); CHECK_EQ(r.error,Error::None);
        CHECK(r.applied && r.order.has_value()); return *r.order;
    }
    const Order& order(OrderId id) const { return core.state().orders.at(id); }
};
} // namespace fixture
