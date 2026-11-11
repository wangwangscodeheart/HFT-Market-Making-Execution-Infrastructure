#pragma once
#include "local_sim_scenario.hpp"
#include "p03/local_quote_runner.hpp"
namespace p03::example {
inline LocalQuoteRunnerConfig quote_runner_config(){
    auto source=standard_filled_order();auto c=source.config;c.trade_evidence_mode=TradeEvidenceMode::Simulator;
    const BusinessScope scope{c.anchor.account,c.contract.key(),c.anchor.day,c.anchor.epoch};
    return {c,simulator_config(c),{scope,c.strategy,EvidenceOrigin::LocalFixture,"fixture.book",1,10,1,10},128,4096};
}
inline MarketPulse runner_pulse(const LocalQuoteRunnerConfig& c,std::uint64_t sequence,LogicalTime at,Ticks bid,Ticks ask,bool healthy=true){
    return {{c.strategy.scope,EvidenceOrigin::LocalFixture,"fixture.book",1,sequence,at,bid,ask,10,10,healthy},
        {c.core.contract.day,9*3600+60,c.core.contract.day}};
}
inline bool run_quote_scenario(LocalQuoteRunner& run,const LocalQuoteRunnerConfig& c){
    bool ok=run.step(runner_pulse(c,1,1,499,501),1)==Error::None;
    ok=(run.step(runner_pulse(c,2,2,498,502),2)==Error::None)&&ok;
    ok=(run.step(LocalFillInput{{c.core.instance,c.core.next_order+2},"runner-export-fill",1,498},3)==Error::None)&&ok;
    ok=(run.step(runner_pulse(c,3,4,498,502,false),4)==Error::Blocked)&&ok;
    ok=(run.step(StopLocalQuotes{},5)==Error::None)&&ok;
    const auto& frames=run.trace();if(frames.size()!=5)return false;
    ok=ok && frames[0].orders.size()==2 && frames[1].orders.size()==4 &&
        frames[2].positions==std::array<Lots,4>{1,0,0,0} && frames[4].mode==CoreMode::Killed;
    for(const auto& f:frames){
        ok=ok && f.completed && f.peer.has_value() && f.positions==f.peer->positions;
        if(f.ordinal>=4)for(const auto& order:f.orders)ok=ok && order.parts.held()==0;
    }
    return ok;
}
inline bool run_quote_expiry_scenario(LocalQuoteRunner& run,const LocalQuoteRunnerConfig& c){
    const auto boundary=1+c.strategy.max_age;
    bool ok=run.step(runner_pulse(c,1,1,499,501),1)==Error::None;
    ok=(run.step(LocalQuoteTimer{},boundary)==Error::None)&&ok;
    ok=(run.step(LocalQuoteTimer{},boundary+1)==Error::StaleMark)&&ok;
    ok=(run.step(LocalQuoteTimer{},boundary+2)==Error::StaleMark)&&ok;
    ok=(run.step(runner_pulse(c,2,boundary+3,499,501),boundary+3)==Error::None)&&ok;
    ok=(run.step(StopLocalQuotes{},boundary+4)==Error::None)&&ok;
    const auto& frames=run.trace();if(frames.size()!=6)return false;
    ok=ok && frames[1].calls.empty() && frames[2].calls.size()==6 && frames[3].calls.empty() && frames[4].orders.size()==4;
    for(const auto& f:frames)ok=ok && f.completed && f.peer && f.positions==f.peer->positions && f.positions==std::array<Lots,4>{};
    for(const auto i:{2U,3U,5U})for(const auto& o:frames[i].orders)ok=ok && o.parts.held()==0;
    return ok && frames.back().mode==CoreMode::Killed;
}
inline bool run_quote_query_scenario(LocalQuoteRunner& run,const LocalQuoteRunnerConfig& c){
    bool ok=run.step(runner_pulse(c,1,1,499,501),1)==Error::None;
    ok=(run.step(LocalDeferredFillInput{{{c.core.instance,c.core.next_order},"export-query-gap",1,499}},2)==Error::None)&&ok;
    ok=(run.step(runner_pulse(c,2,3,499,501),3)==Error::Blocked)&&ok;
    ok=(run.step(LocalTradeQueryInput{{c.core.instance,100},true},4)==Error::Blocked)&&ok;
    ok=(run.step(LocalTradeQueryInput{{c.core.instance,101},false},5)==Error::None)&&ok;
    ok=(run.step(LocalHeldReportInput{0},6)==Error::Duplicate)&&ok;
    ok=(run.step(runner_pulse(c,3,7,499,501),7)==Error::None)&&ok;
    ok=(run.step(StopLocalQuotes{},8)==Error::None)&&ok;
    const auto& f=run.trace();if(f.size()!=8)return false;
    ok=ok && f[1].positions[0]==0 && f[1].peer->positions[0]==1 && f[1].pending_reports==1 &&
        f[3].calls.empty() && f[3].canonical==f[2].canonical && f[3].pending_reports==1 &&
        f[4].pending_reports==0 && f[4].positions[0]==1 && f[5].calls.size()==1 &&
        f[5].calls[0].result.error==Error::Duplicate && f[6].orders.size()==4;
    for(const auto& call:f[2].calls)ok=ok && !std::holds_alternative<SubmitOrder>(call.input);
    for(const auto& frame:f)ok=ok && frame.completed && frame.peer.has_value();
    for(const auto& o:f.back().orders)ok=ok && o.parts.held()==0;
    return ok && f.back().mode==CoreMode::Killed && f.back().positions==f.back().peer->positions && f.back().positions[0]==1;
}
inline bool run_quote_retry_scenario(LocalQuoteRunner& run,const LocalQuoteRunnerConfig& c){
    bool ok=run.step(runner_pulse(c,1,1,499,501),1)==Error::None;
    ok=(run.step(LocalDeferredFillInput{{{c.core.instance,c.core.next_order},"retry-buy",1,499}},2)==Error::None)&&ok;
    ok=(run.step(LocalDeferredFillInput{{{c.core.instance,c.core.next_order+1},"retry-sell",1,501}},3)==Error::None)&&ok;
    ok=(run.step(LocalReportFailureInput{0},4)==Error::None)&&ok;
    ok=(run.step(LocalTradeQueryInput{{c.core.instance,100},false},5)==Error::InjectedFailure)&&ok;
    ok=(run.step(runner_pulse(c,2,6,499,501),6)==Error::Blocked)&&ok;
    ok=(run.step(LocalRetryEvidenceInput{},7)==Error::None)&&ok;
    ok=(run.step(runner_pulse(c,3,8,499,501),8)==Error::Blocked)&&ok;
    ok=(run.step(LocalTradeQueryInput{{c.core.instance,101},false},9)==Error::None)&&ok;
    ok=(run.step(StopLocalQuotes{},10)==Error::None)&&ok;
    const auto& f=run.trace();if(f.size()!=10)return false;
    ok=ok && f[4].pending_delivery==2 && f[4].pending_reports==2 && f[4].calls.size()==1 &&
        f[4].positions==std::array<Lots,4>{} && f[5].calls.empty() &&
        f[6].pending_delivery==0 && f[6].pending_reports==0 && f[6].query_blocked &&
        f[7].calls.empty() && f[8].calls.size()==2 && !f[8].query_blocked;
    for(const auto& call:f[8].calls)ok=ok && call.result.error==Error::Duplicate;
    for(const auto& frame:f)ok=ok && frame.completed && !frame.delivery_loss && frame.peer.has_value();
    return ok && f.back().mode==CoreMode::Killed && f.back().positions==std::array<Lots,4>{1,0,1,0} && f.back().positions==f.back().peer->positions;
}
inline LocalQuoteRunnerConfig quote_overflow_config(){
    auto c=quote_runner_config();c.core.anchor.booked={10,0,0,0};c.core.risk.anchor_margin=1000;c.core.capacity.orders=1;
    c.peer=simulator_config(c.core);c.peer.max_trades=1;return c;
}
inline bool run_quote_overflow_scenario(LocalQuoteRunner& run,const LocalQuoteRunnerConfig& c){
    bool ok=run.step(runner_pulse(c,1,1,499,501),1)==Error::None;
    ok=(run.step(LocalReportFailureInput{0},2)==Error::None)&&ok;
    ok=(run.step(LocalFillInput{{c.core.instance,c.core.next_order},"overflow-export-close",1,501},3)==Error::InjectedFailure)&&ok;
    ok=(run.step(LocalTradeQueryInput{{c.core.instance,1},false,true},4)==Error::Blocked)&&ok;
    ok=(run.step(LocalTradeQueryInput{{c.core.instance,2},false,true},5)==Error::Blocked)&&ok;
    ok=(run.step(LocalTradeQueryInput{{c.core.instance,3},false,true},6)==Error::Capacity)&&ok;
    ok=(run.step(LocalRetryEvidenceInput{},7)==Error::None)&&ok;
    ok=(run.step(LocalTradeQueryInput{{c.core.instance,4},false},8)==Error::Blocked)&&ok;
    ok=(run.step(runner_pulse(c,2,9,499,501),9)==Error::Blocked)&&ok;
    ok=(run.step(StopLocalQuotes{},10)==Error::None)&&ok;
    const auto& f=run.trace();if(f.size()!=10)return false;
    ok=ok && f[2].pending_delivery==1 && f[2].positions[0]==10 && f[2].peer->positions[0]==9 &&
        f[4].pending_delivery==3 && f[5].pending_delivery==3 && f[5].delivery_loss &&
        f[6].pending_delivery==0 && f[6].positions[0]==9 && f[7].query_blocked && f[8].calls.empty();
    for(std::size_t i=5;i<f.size();++i)ok=ok && f[i].delivery_loss && f[i].query_blocked;
    for(const auto& frame:f)ok=ok && frame.completed;
    return ok && f.back().mode==CoreMode::Killed && f.back().positions==f.back().peer->positions;
}
struct RunnerAdmissionCheck {
    LogicalTime at{};Error expected{Error::None},actual{Error::None};
    std::size_t frames_before{},frames_after{};bool core_unchanged{};
};
inline LocalQuoteRunnerConfig quote_cleanup_config(){auto c=quote_runner_config();c.max_inputs=3;c.cleanup_inputs=3;return c;}
inline bool run_quote_cleanup_scenario(LocalQuoteRunner& run,const LocalQuoteRunnerConfig& c,std::vector<RunnerAdmissionCheck>* checks=nullptr){
    auto rejected=[&](LocalQuoteInput input,LogicalTime at,Error expected){
        const auto before=run.core().canonical_state();const auto count=run.trace().size();const auto actual=run.step(std::move(input),at);
        RunnerAdmissionCheck check{at,expected,actual,count,run.trace().size(),before==run.core().canonical_state()};
        if(checks)checks->push_back(check);
        return actual==expected && check.frames_before==check.frames_after && check.core_unchanged;
    };
    bool ok=run.step(runner_pulse(c,1,1,499,501),1)==Error::None;
    ok=(run.step(LocalReportFailureInput{0},2)==Error::None)&&ok;
    ok=(run.step(LocalFillInput{{c.core.instance,c.core.next_order},"cleanup-export-fill",1,499},3)==Error::InjectedFailure)&&ok;
    ok=rejected(runner_pulse(c,2,4,499,501),4,Error::Capacity)&&ok;
    ok=(run.step(LocalRetryEvidenceInput{},5)==Error::None)&&ok;
    ok=(run.step(StopLocalQuotes{},6)==Error::None)&&ok;
    ok=(run.step(LocalTradeQueryInput{{c.core.instance,100},false},7)==Error::None)&&ok;
    ok=rejected(runner_pulse(c,3,8,499,501),8,Error::Halted)&&ok;
    ok=rejected(LocalRetryEvidenceInput{},9,Error::Capacity)&&ok;
    const auto& f=run.trace();if(f.size()!=6)return false;
    for(std::size_t i=0;i<f.size();++i)ok=ok && f[i].completed && f[i].cleanup_lane==(i>=3);
    for(const auto& o:f.back().orders)ok=ok && o.parts.held()==0;
    return ok && f[2].pending_delivery==1 && f[3].pending_delivery==0 && f.back().mode==CoreMode::Killed &&
        f.back().positions==std::array<Lots,4>{1,0,0,0} && f.back().positions==f.back().peer->positions && run.stopped() && run.cleanup_exhausted();
}
inline LocalQuoteRunnerConfig quote_late_fill_config(){
    auto c=quote_runner_config();c.strategy.quantity=3;c.max_inputs=2;c.cleanup_inputs=8;return c;
}
inline bool run_quote_late_fill_scenario(LocalQuoteRunner& run,const LocalQuoteRunnerConfig& c){
    bool ok=run.step(runner_pulse(c,1,1,499,501),1)==Error::None;
    ok=(run.step(LocalDeferredFillInput{{{c.core.instance,c.core.next_order},"late-after-kill",1,499}},2)==Error::None)&&ok;
    ok=(run.step(StopLocalQuotes{},3)==Error::None)&&ok;
    ok=(run.step(StopLocalQuotes{},4)==Error::None)&&ok;
    ok=(run.step(LocalHeldReportInput{0},5)==Error::None)&&ok;
    ok=(run.step(LocalTradeQueryInput{{c.core.instance,100},false},6)==Error::None)&&ok;
    ok=(run.step(LocalHeldReportInput{0},7)==Error::Duplicate)&&ok;
    const auto& f=run.trace();if(f.size()!=7)return false;
    ok=ok && f[2].orders[0].parts==QuantityParts{0,1,0,2,0} && f[2].core_required==102 &&
        f[3].calls.size()==1 && std::holds_alternative<KillRequested>(f[3].calls[0].input) &&
        f[4].orders[0].parts==QuantityParts{1,0,0,2,0} && f[4].core_required==102 &&
        f[5].calls.size()==1 && f[5].calls[0].result.error==Error::Duplicate;
    for(std::size_t i=2;i<f.size();++i)ok=ok && f[i].cleanup_lane && f[i].mode==CoreMode::Killed;
    for(const auto& frame:f)ok=ok && frame.completed && frame.peer.has_value();
    return ok && run.stopped() && f.back().positions==std::array<Lots,4>{1,0,0,0} && f.back().positions==f.back().peer->positions;
}
} // namespace p03::example
