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
} // namespace p03::example
