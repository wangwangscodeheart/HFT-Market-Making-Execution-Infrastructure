#include "p03/funds.hpp"
#include "../examples/json_output.hpp"
#include <algorithm>
#include <chrono>
#include <iostream>
namespace {
using namespace p03;using Clock=std::chrono::steady_clock;
constexpr Money cap=1000000,buffer=7;
FundsView nested(const FundsLedger& ledger){
    FundsView v;
    for(const auto& [id,r]:ledger.resources()){
        v.local_required=checked_add(v.local_required,r.total());
        for(const auto& proof:ledger.observation()->coverage)
            if(proof.resource==id && proof.status==CoverageStatus::Proven)v.proven_cover=checked_add(v.proven_cover,std::min(proof.amount,r.margin()));
    }
    v.uncovered=v.local_required-v.proven_cover;v.local_ok=checked_add(v.local_required,buffer)<=cap;v.has_external=true;
    v.external_budget=checked_add(checked_add(ledger.observation()->external_available,-v.uncovered),-buffer);v.external_ok=v.external_budget>=0;return v;
}
bool same(const FundsView& a,const FundsView& b){
    return a.local_required==b.local_required && a.proven_cover==b.proven_cover && a.uncovered==b.uncovered && a.external_budget==b.external_budget &&
        a.has_external==b.has_external && a.local_ok==b.local_ok && a.external_ok==b.external_ok;
}
template<class F> FundsView timed(F&& fn,std::vector<std::uint64_t>& samples,bool measured){
    const auto a=Clock::now();const auto result=fn();const auto b=Clock::now();
    const auto ns=std::chrono::duration_cast<std::chrono::nanoseconds>(b-a).count();if(ns<0)throw std::logic_error("steady clock reversed");
    if(measured)samples.push_back(static_cast<std::uint64_t>(ns));return result;
}
void distribution(std::vector<std::uint64_t> samples){
    std::cout<<"{\"samples_ns\":";example::json_numbers(std::cout,samples);std::sort(samples.begin(),samples.end());
    std::cout<<",\"min_ns\":"<<samples.front()<<",\"p50_ns\":"<<samples.at(249)<<",\"p99_ns\":"<<samples.at(494)<<",\"max_ns\":"<<samples.back()<<'}';
}
}
int main(int argc,char**){
    if(argc!=1)return 2;
    std::cout<<"{\"schema\":\"p03.funds-view-benchmark.v1\",\"origin\":\"LocalFixture\",\"warmup_pairs_per_profile\":50,\"measured_pairs_per_profile\":500,\"measured_calls\":4000,\"unit\":\"nanoseconds\",\"clock\":\"steady_clock\",\"alternating_pair_order\":true,\"setup_included\":false,\"external_checks_included\":false,\"timer_overhead_subtracted\":false,\"gateway_included\":false,\"profiles\":[";
    bool first=true;for(const std::size_t count:{0U,8U,32U,64U}){
        FundsLedger ledger(cap,buffer,"fixture.account",TradingDay{20261105},1);
        std::vector<FundResource> resources;std::vector<CoverageProof> proofs;
        for(std::size_t i=0;i<count;++i){const ReservationId id{7,static_cast<std::uint64_t>(i+1)};
            resources.push_back({id,60,40,static_cast<Money>(i+1)});
            proofs.push_back({id,i%3==0?CoverageStatus::Proven:i%3==1?CoverageStatus::Unknown:CoverageStatus::Excluded,60,"deduction-"+std::to_string(i)});
        }
        std::reverse(proofs.begin(),proofs.end());ledger.replace_resources(resources);
        FundsObservation observation{"fixture.account",TradingDay{20261105},1,1,100000,EvidenceOrigin::LocalFixture,std::move(proofs),
            {"fixture.query","p03.fixture.v1","fixture.account-instrument","FIXTURE_UNIT",0,1}};
        if(ledger.observe(observation)!=Error::None)return 1;
        std::vector<std::uint64_t> fast,reference;fast.reserve(500);reference.reserve(500);
        for(std::size_t i=0;i<550;++i){
            FundsView a,b;if(i%2==0){a=timed([&]{return ledger.view();},fast,i>=50);b=timed([&]{return nested(ledger);},reference,i>=50);}
            else {b=timed([&]{return nested(ledger);},reference,i>=50);a=timed([&]{return ledger.view();},fast,i>=50);}
            if(!same(a,b))return 1;
        }
        const auto v=ledger.view();if(!first)std::cout<<',';first=false;
        std::cout<<"{\"resources\":"<<count<<",\"proofs\":"<<count<<",\"local_required\":"<<v.local_required<<",\"proven_cover\":"<<v.proven_cover<<",\"uncovered\":"<<v.uncovered<<",\"external_budget\":"<<v.external_budget<<",\"optimized\":";
        distribution(fast);std::cout<<",\"nested_reference\":";distribution(reference);std::cout<<'}';
    }
    std::cout<<"],\"passed\":true}\n";
}
