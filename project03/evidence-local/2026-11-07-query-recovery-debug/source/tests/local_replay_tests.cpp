#include "fixture.hpp"
#include "../examples/local_replay_scenario.hpp"
using namespace fixture;
TEST_CASE(REPLAY_query_recovers_missing_fill_before_late_push_without_double_booking){
    const auto query=example::delayed_cancel_replay(true),push=example::delayed_cancel_replay(false);
    CHECK(query.passed && push.passed);CHECK_EQ(query.frames.size(),10U);
    CHECK_EQ(query.frames[6].parts,(QuantityParts{0,2,0,8,0}));
    CHECK_EQ(query.frames[7].action,"query_recovers_missing_trade");CHECK_EQ(query.frames[7].error,Error::None);
    CHECK_EQ(query.frames[7].report_id,0U);CHECK_EQ(query.frames[7].parts,(QuantityParts{2,0,0,8,0}));
    CHECK_EQ(query.frames[8].action,"late_push_after_query");CHECK_EQ(query.frames[8].error,Error::Duplicate);
    CHECK_EQ(query.frames[9].error,Error::Duplicate);
    CHECK_EQ(query.frames.back().core_positions,push.frames.back().core_positions);
    CHECK_EQ(query.frames.back().parts,push.frames.back().parts);
    for(const auto& frame:query.frames)CHECK(frame.passed);
}
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
