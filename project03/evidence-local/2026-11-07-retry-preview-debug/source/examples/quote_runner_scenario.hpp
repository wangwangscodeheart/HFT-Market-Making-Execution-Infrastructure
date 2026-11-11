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
} // namespace p03::example
