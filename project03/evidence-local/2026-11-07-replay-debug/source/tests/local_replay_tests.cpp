#include "fixture.hpp"
#include "../examples/local_replay_scenario.hpp"
using namespace fixture;
TEST_CASE(REPLAY_script_assertions_cover_peer_core_divergence_and_convergence){
    const auto run=example::delayed_cancel_replay();CHECK(run.passed);CHECK_EQ(run.frames.size(),10U);
    for(const auto& f:run.frames)CHECK(f.passed);
    CHECK_EQ(run.frames[2].peer_positions[0],2);CHECK_EQ(run.frames[2].core_positions[0],0);
    CHECK_EQ(run.frames[6].parts.suspense,2);CHECK_EQ(run.frames[6].parts.released,8);
    CHECK_EQ(run.frames[7].parts.booked,2);CHECK_EQ(run.frames[7].parts.suspense,0);
    CHECK_EQ(run.frames[8].error,Error::Duplicate);CHECK_EQ(run.frames[9].error,Error::Duplicate);
}
TEST_CASE(REPLAY_repeated_runs_have_identical_states_and_delivery_sequences){
    const auto a=example::delayed_cancel_replay(),b=example::delayed_cancel_replay();CHECK(a.passed && b.passed);
    CHECK_EQ(a.frames.size(),b.frames.size());
    for(std::size_t i=0;i<a.frames.size();++i){
        const auto& x=a.frames[i];const auto& y=b.frames[i];
        CHECK_EQ(x.action,y.action);CHECK_EQ(x.at,y.at);CHECK_EQ(x.peer_revision,y.peer_revision);
        CHECK_EQ(x.report_id,y.report_id);CHECK_EQ(x.ingress,y.ingress);CHECK_EQ(x.sequence,y.sequence);
        CHECK_EQ(x.error,y.error);CHECK_EQ(x.peer_positions,y.peer_positions);CHECK_EQ(x.core_positions,y.core_positions);
        CHECK_EQ(x.parts,y.parts);
    }
}
