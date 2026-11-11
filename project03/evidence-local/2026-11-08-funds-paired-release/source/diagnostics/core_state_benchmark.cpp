#include "../examples/demo_scenario.hpp"
#include "../examples/json_output.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
namespace {
using namespace p03;
using Clock=std::chrono::steady_clock;
struct Profile {std::size_t orders;bool filled;std::vector<std::uint64_t> samples;};
}
int main(int argc,char**){
    if(argc!=1)return 2;
    constexpr std::size_t warmup=20,measured=100;
    std::vector<Profile> profiles;
    for(const bool filled:{false,true})for(const std::size_t count:{0U,8U,32U,63U}){
        auto scenario=example::standard_filled_order();scenario.config.risk.max_mark_age=1000000;
        CoreLoop core(scenario.config);LogicalTime at=0;
        if(core.step(ReferenceMark{500},at++).error!=Error::None)return 1;
        auto input=std::get<SubmitOrder>(scenario.inputs.at(1));input.intent.quantity=1;
        for(std::size_t i=0;i<count;++i){
            input.client_id={"fixture.state-benchmark",std::to_string(i)};
            const auto accepted=core.step(input,at++);if(accepted.error!=Error::None || !accepted.order)return 1;
            if(filled){
                auto report=std::get<TradeReport>(scenario.inputs.at(3));report.trade.order=*accepted.order;
                report.trade.key.trade_id="state-fill-"+std::to_string(i);report.trade.quantity=1;
                if(core.step(report,at++).error!=Error::None)return 1;
            }
        }
        const auto expected_positions=core.state().positions.booked();
        const auto expected_funds=core.state().funds.view().local_required;
        Profile profile{count,filled,{}};profile.samples.reserve(measured);
        for(std::size_t i=0;i<warmup+measured;++i){
            const BusinessEvent mark=ReferenceMark{500};const auto now=at++;
            const auto begin=Clock::now();const auto result=core.step(mark,now);const auto end=Clock::now();
            const auto ns=std::chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count();
            if(ns<0 || result.error!=Error::None || !result.applied || !core.invariant() ||
               core.state().orders.size()!=count || core.state().executions.records().size()!=(filled?count:0) ||
               core.state().positions.booked()!=expected_positions || core.state().funds.view().local_required!=expected_funds)return 1;
            if(i>=warmup)profile.samples.push_back(static_cast<std::uint64_t>(ns));
        }
        Lots held=0;for(const auto& [id,order]:core.state().orders){(void)id;held=checked_add(held,order.parts().held());}
        if(expected_positions!=std::array<Lots,4>{filled?static_cast<Lots>(count):0,0,0,0} || held!=(filled?0:static_cast<Lots>(count)))return 1;
        profiles.push_back(std::move(profile));
    }
    std::cout<<"{\"schema\":\"p03.core-state-benchmark.v1\",\"origin\":\"LocalFixture\",\"workload\":\"controlled_seeded_state_reference_mark\",\"unit\":\"nanoseconds\",\"warmup_per_profile\":20,\"samples_per_profile\":100,\"measured_steps\":800,\"setup_included\":false,\"external_checks_included\":false,\"internal_copy_validation_allocation_included\":true,\"gateway_included\":false,\"passed\":true,\"profiles\":[";
    bool first=true;for(auto& p:profiles){if(!first)std::cout<<',';first=false;
        std::cout<<"{\"state\":\""<<(p.filled?"filled":"working")<<"\",\"orders\":"<<p.orders<<",\"trades\":"<<(p.filled?p.orders:0)<<",\"booked_lots\":"<<(p.filled?p.orders:0)<<",\"held_lots\":"<<(p.filled?0:p.orders)<<",\"samples_ns\":";
        example::json_numbers(std::cout,p.samples);std::sort(p.samples.begin(),p.samples.end());
        std::cout<<",\"min_ns\":"<<p.samples.front()<<",\"p50_ns\":"<<p.samples.at(49)<<",\"p99_ns\":"<<p.samples.at(98)<<",\"max_ns\":"<<p.samples.back()<<'}';
    }
    std::cout<<"]}\n";
}
