#include "demo_scenario.hpp"
#include <iostream>

int main(int argc,char** argv) {
    using namespace p03;
    const auto scenario=example::select_scenario(argc,argv);
    StepHarness harness(scenario.config);
    const auto id=scenario.order;
    LogicalTime time=0;
    for(const auto& e:scenario.inputs) if(harness.schedule(time++,e)!=Error::None) return 1;
    if(harness.run()!=Error::None) return 1;
    std::cout<<"PROJECT03 M1A synthetic business inputs; no broker, exchange simulator or outbound send.\n";
    for(const auto& s:harness.trace()) std::cout<<"step="<<s.result.core_sequence<<" error="<<error_name(s.result.error)
        <<" applied="<<s.result.applied<<'\n'<<s.canonical_state<<'\n';
    const auto& order=harness.core().state().orders.at(id); const auto p=order.parts();
    std::cout<<"FINAL Q="<<order.intent().quantity<<" B="<<p.booked<<" H="<<p.suspense<<" P="<<p.possible<<" L="<<p.released<<" V="<<p.unresolved
        <<" booked_long_today="<<harness.core().state().positions.booked()[0]<<'\n';
    return p==scenario.expected && harness.core().invariant() ? 0 : 1;
}
