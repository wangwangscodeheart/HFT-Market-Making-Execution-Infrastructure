#include "p03/step_harness.hpp"
#include <iostream>

int main() {
    using namespace p03;
    const TradingDay day{20261105};
    Contract contract{"rb_FIXTURE","RB","SHFE","fixture.contract.v1",day,100,1000,1,1,10,1,true};
    TradingCalendar calendar{"fixture.calendar.v1",{{day,9*3600,10*3600,day}}};
    PositionAnchor anchor{"fixture.account","rb_FIXTURE",day,1,"fixture.initial.empty",{}};
    StepHarness harness({7,1,contract,calendar,anchor,{},{}});
    const OrderId id{7,1};
    const EvidenceStamp stamp{EvidenceOrigin::LocalFixture,"fixture.demo",1,{},false};
    const EvidenceHeader header{id,"fixture.account",day,stamp};
    const OrderIntent intent{"fixture.account","rb_FIXTURE",day,Side::Buy,Offset::Open,499,3};
    auto trade=[&](std::string tid) {
        return TradeReport{{{"fixture.account",day,"SHFE","p03.fixture.day.v1",std::move(tid)},
            id,"rb_FIXTURE",Side::Buy,Offset::Open,1,499},stamp,Channel::Push};
    };
    const std::vector<BusinessEvent> inputs{ReferenceMark{500},SubmitOrder{intent,{day,9*3600+1,day}},
        trade("t1"),RequestCancel{id},TerminalReport{header,{TerminalKind::Canceled,2}},trade("t2"),trade("t2"),
        RequestFutureOperation{FutureOperation::SendOrder}};
    LogicalTime time=0;
    for(const auto& e:inputs) if(harness.schedule(time++,e)!=Error::None) return 1;
    if(harness.run()!=Error::None) return 1;
    std::cout<<"PROJECT03 M1A synthetic business inputs; no broker, exchange simulator or outbound send.\n";
    for(const auto& s:harness.trace()) std::cout<<"step="<<s.result.core_sequence<<" error="<<error_name(s.result.error)
        <<" applied="<<s.result.applied<<'\n'<<s.canonical_state<<'\n';
    const auto& order=harness.core().state().orders.at(id); const auto p=order.parts();
    std::cout<<"FINAL Q=3 B="<<p.booked<<" H="<<p.suspense<<" P="<<p.possible<<" L="<<p.released<<" V="<<p.unresolved
        <<" booked_long_today="<<harness.core().state().positions.booked()[0]<<'\n';
    return p==QuantityParts{2,0,0,1,0} && harness.core().invariant() ? 0 : 1;
}
