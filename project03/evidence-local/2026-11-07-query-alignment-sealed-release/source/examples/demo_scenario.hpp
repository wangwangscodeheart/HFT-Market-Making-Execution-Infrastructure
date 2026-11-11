#pragma once
#include "p03/step_harness.hpp"
#include <string_view>

// Shared deterministic inputs for the text demo and offline snapshot export.
namespace p03::example {
struct Scenario {
    CoreConfig config;
    OrderId order;
    std::vector<BusinessEvent> inputs;
    QuantityParts expected;
    std::string_view key, source_url, source_ref;
};
inline Scenario cancellation_race() {
    const TradingDay day{20261105};
    Contract contract{"rb_FIXTURE","RB","SHFE","fixture.contract.v1",day,100,1000,1,1,10,1,true};
    TradingCalendar calendar{"fixture.calendar.v1",{{day,9*3600,10*3600,day}}};
    PositionAnchor anchor{"fixture.account","rb_FIXTURE",day,1,"fixture.initial.empty",{}};
    const OrderId id{7,1};
    const EvidenceStamp stamp{EvidenceOrigin::LocalFixture,"fixture.demo",1,{},false};
    const EvidenceHeader header{id,"fixture.account",day,stamp};
    const OrderIntent intent{"fixture.account","rb_FIXTURE",day,Side::Buy,Offset::Open,499,3};
    auto trade=[&](std::string tid) {
        return TradeReport{{{"fixture.account",day,"SHFE","p03.fixture.day.v1",std::move(tid)},
            id,"rb_FIXTURE",Side::Buy,Offset::Open,1,499},stamp,Channel::Push};
    };
    return {{7,1,std::move(contract),std::move(calendar),std::move(anchor),{},{}}, id,
        {ReferenceMark{500},SubmitOrder{intent,{day,9*3600+1,day},ClientOrderId{"fixture.demo","cancel-race.request"}},trade("t1"),RequestCancel{id},
         TerminalReport{header,{TerminalKind::Canceled,2}},trade("t2"),trade("t2"),
         RequestFutureOperation{FutureOperation::SendOrder}},
        {2,0,0,1,0},"race","","Local cancellation-race contract regression; not a historical incident"};
}
inline Scenario standard_filled_order() {
    auto scenario=cancellation_race();
    const auto day=scenario.config.contract.day;
    const EvidenceStamp stamp{EvidenceOrigin::LocalFixture,"fixture.standard.a1a",1,{},false};
    const EvidenceHeader header{scenario.order,scenario.config.anchor.account,day,stamp};
    const OrderIntent intent{scenario.config.anchor.account,scenario.config.contract.instrument,
        day,Side::Buy,Offset::Open,499,10};
    auto fill=[&](std::string key,Lots quantity){
        return TradeReport{{{scenario.config.anchor.account,day,"SHFE","p03.fixture.day.v1",std::move(key)},
            scenario.order,scenario.config.contract.instrument,Side::Buy,Offset::Open,quantity,499},stamp,Channel::Push};
    };
    // Official A.1.a quantity sequence 10000 / (2000,1000,7000), scaled exactly by 1000.
    // Account, RB contract, price, opening side, calendar and funds remain local fixtures.
    scenario.inputs={ReferenceMark{500},SubmitOrder{intent,{day,9*3600+1,day},ClientOrderId{"fixture.standard","A1a.request"}},WorkingReport{header},
        fill("a1a.fill.1",2),fill("a1a.fill.2",1),fill("a1a.fill.3",7)};
    scenario.expected={10,0,0,0,0};
    scenario.key="normal";
    scenario.source_url="https://www.fixtrading.org/wp-content/uploads/download-manager-files/Order-State-Changes.pdf";
    scenario.source_ref="FIX Order State Changes, July 2020, A.1.a, accepted/filled branch; quantity scale 1:1000";
    return scenario;
}
inline Scenario select_scenario(int argc,char** argv) {
    if(argc==1)return cancellation_race();
    if(argc==2 && std::string_view(argv[1])=="--normal")return standard_filled_order();
    throw std::invalid_argument("usage: executable [--normal]");
}
} // namespace p03::example
