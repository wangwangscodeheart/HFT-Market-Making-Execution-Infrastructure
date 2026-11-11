#include "../examples/demo_scenario.hpp"
#include "../examples/json_output.hpp"
#include <algorithm>
#include <array>
#include <chrono>
#include <iostream>
namespace {
using Clock=std::chrono::steady_clock;
std::uint64_t elapsed(Clock::time_point a,Clock::time_point b){
    const auto ns=std::chrono::duration_cast<std::chrono::nanoseconds>(b-a).count();
    if(ns<0)throw std::logic_error("steady clock moved backwards");
    return static_cast<std::uint64_t>(ns);
}
void samples(std::vector<std::uint64_t> values){
    p03::example::json_numbers(std::cout,values);std::sort(values.begin(),values.end());
    const auto rank=[&](std::size_t p){return values.at((values.size()*p+99)/100-1);};
    std::cout<<",\"min_ns\":"<<values.front()<<",\"p50_ns\":"<<rank(50)<<",\"p99_ns\":"<<rank(99)<<",\"max_ns\":"<<values.back();
}
}
int main(int argc,char**){
    if(argc!=1)return 2;
    constexpr std::size_t warmup=50,cycles=500;
    const auto scenario=p03::example::standard_filled_order();
    std::array<std::vector<std::uint64_t>,6> durations;
    for(auto& values:durations)values.reserve(cycles);
    std::string baseline;
    for(std::size_t cycle=0;cycle<warmup+cycles;++cycle){
        p03::CoreLoop core(scenario.config); // Construction is outside every timed interval.
        for(std::size_t i=0;i<scenario.inputs.size();++i){
            const auto begin=Clock::now();const auto result=core.step(scenario.inputs[i],static_cast<p03::LogicalTime>(i));const auto end=Clock::now();
            if(result.error!=p03::Error::None || !result.applied || !core.invariant())return 1;
            if(cycle>=warmup)durations.at(i).push_back(elapsed(begin,end));
        }
        if(core.state().orders.at(scenario.order).parts()!=scenario.expected || core.state().positions.booked()!=std::array<p03::Lots,4>{10,0,0,0})return 1;
        const auto canonical=core.canonical_state();if(cycle==0)baseline=canonical;else if(canonical!=baseline)return 1;
    }
    std::vector<std::uint64_t> overhead;overhead.reserve(cycles);
    for(std::size_t i=0;i<cycles;++i){const auto a=Clock::now();const auto b=Clock::now();overhead.push_back(elapsed(a,b));}
    std::cout<<"{\"schema\":\"p03.core-step-benchmark.v1\",\"scenario\":\"FIX_A1a_local_scaled\",\"origin\":\"LocalFixture\",\"warmup_cycles\":"<<warmup
        <<",\"measured_cycles\":"<<cycles<<",\"events_per_cycle\":6,\"measured_steps\":3000,\"clock\":\"steady_clock\",\"unit\":\"nanoseconds\",\"clock_period_num\":"<<Clock::period::num
        <<",\"clock_period_den\":"<<Clock::period::den<<",\"nearest_rank_percentiles\":true,\"timer_overhead_subtracted\":false,\"core_internal_allocation_included\":true,\"construction_included\":false,\"external_invariant_and_canonical_checks_included\":false,\"single_thread\":true,\"gateway_included\":false,\"passed\":true,\"final_positions\":[10,0,0,0],\"timer_overhead\":{\"samples_ns\":";
    samples(overhead);std::cout<<"},\"stages\":[";
    const std::array<const char*,6> names{"ReferenceMark","SubmitOrder","WorkingReport","Fill2","Fill1","Fill7"};
    for(std::size_t i=0;i<durations.size();++i){if(i)std::cout<<',';std::cout<<"{\"event\":\""<<names[i]<<"\",\"samples_ns\":";samples(durations[i]);std::cout<<'}';}
    std::cout<<"]}\n";
}
