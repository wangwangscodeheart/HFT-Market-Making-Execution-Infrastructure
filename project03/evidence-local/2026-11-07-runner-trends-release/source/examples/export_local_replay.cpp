#include "local_replay_scenario.hpp"
#include <iostream>
int main(int argc,char** argv){
    using namespace p03;
    if(argc>2 || (argc==2 && std::string_view(argv[1])!="--query-first"))return 2;
    const bool query_first=argc==2;
    const auto run=example::delayed_cancel_replay(query_first);
    std::cout<<"{\"schema\":\"p03.local-replay.v1\",\"scenario\":\""<<(query_first?"query-first-cancel":"delayed-fill-cancel")<<"\",\"origin\":\"LocalSimulator\","
        "\"historical_trace\":false,\"time_unit\":\"logical_step\",\"blocked_queries\":"<<run.blocked_queries<<",\"passed\":"<<(run.passed?"true":"false")<<",\"frames\":[";
    bool first=true;
    for(const auto& f:run.frames){
        if(!first)std::cout<<',';first=false;
        std::cout<<"{\"action\":\""<<f.action<<"\",\"at\":"<<f.at<<",\"peer_revision\":"<<f.peer_revision
            <<",\"report_id\":"<<f.report_id<<",\"ingress\":"<<f.ingress<<",\"core_sequence\":"<<f.sequence
            <<",\"error\":\""<<error_name(f.error)<<"\",\"peer_positions\":[";
        for(std::size_t i=0;i<4;++i){if(i)std::cout<<',';std::cout<<f.peer_positions[i];}
        std::cout<<"],\"core_positions\":[";
        for(std::size_t i=0;i<4;++i){if(i)std::cout<<',';std::cout<<f.core_positions[i];}
        std::cout<<"],\"parts\":["<<f.parts.booked<<','<<f.parts.suspense<<','<<f.parts.possible<<','<<f.parts.released<<','<<f.parts.unresolved
            <<"],\"passed\":"<<(f.passed?"true":"false")<<'}';
    }
    std::cout<<"]}\n";return run.passed?0:1;
}
