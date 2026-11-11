#include "fixture.hpp"
#include "p03/local_delivery.hpp"
#include "../examples/local_sim_scenario.hpp"
using namespace fixture;
namespace {
CoreConfig delivery_config(){auto c=config();c.trade_evidence_mode=TradeEvidenceMode::Simulator;return c;}
SimOrderRequest request(OrderId id){return {id,intent(),StrategyId{"fixture.default.strategy"},{}};}
}
TEST_CASE(DELIVERY_delayed_trade_is_visible_in_peer_before_core_and_replay_is_duplicate){
    Fixture f(delivery_config());LocalSimVenue venue(example::simulator_config(delivery_config()));LocalReportDelivery link;
    const auto id=f.submit();CHECK_EQ(link.capture(venue.receive(request(id))),Error::None);
    CHECK_EQ(link.deliver(f.core,1,2).error,Error::None);
    CHECK_EQ(link.capture(venue.execute_fill(id,"delayed-1",1,499)),Error::None);
    CHECK_EQ(venue.query({7,1}).snapshot->positions[0],1);CHECK_EQ(f.core.state().positions.booked()[0],0);
    CHECK_EQ(link.deliver(f.core,2,10).error,Error::None);CHECK_EQ(f.core.state().positions.booked()[0],1);
    CHECK_EQ(link.deliver(f.core,2,11).error,Error::Duplicate);
    CHECK_EQ(link.attempts().size(),3U);CHECK_EQ(link.attempts()[1].report_id,2U);
    CHECK_EQ(link.attempts()[1].at,10U);CHECK_EQ(link.reports()[1].peer_revision,2U);
    CHECK(f.core.invariant());CHECK(venue.invariant());
}
TEST_CASE(DELIVERY_A07_terminal_precedes_delayed_trade_without_losing_exposure){
    Fixture f(delivery_config());LocalSimVenue venue(example::simulator_config(delivery_config()));LocalReportDelivery link;
    const auto id=f.submit();CHECK_EQ(link.capture(venue.receive(request(id))),Error::None);
    CHECK_EQ(link.deliver(f.core,1,2).error,Error::None);
    CHECK_EQ(link.capture(venue.execute_fill(id,"delayed-2",1,499)),Error::None);
    const auto cancel=f.core.step(RequestCancel{id},3);CHECK(cancel.cancel.has_value());
    CHECK_EQ(link.capture(venue.cancel(id,*cancel.cancel)),Error::None);
    CHECK_EQ(link.deliver(f.core,3,4).error,Error::None);CHECK_EQ(link.deliver(f.core,4,5).error,Error::None);
    CHECK_EQ(f.order(id).parts(),(QuantityParts{0,1,0,2,0}));
    CHECK_EQ(link.deliver(f.core,2,6).error,Error::None);
    CHECK_EQ(f.order(id).parts(),(QuantityParts{1,0,0,2,0}));
    CHECK_EQ(venue.query({7,1}).snapshot->positions,f.core.state().positions.booked());CHECK(f.core.invariant());
}
TEST_CASE(DELIVERY_capture_batch_is_atomic_and_accepts_only_simulator_report_types){
    LocalReportDelivery link(1,1);const auto c=delivery_config();LocalSimVenue venue(example::simulator_config(c));
    const auto accepted=venue.receive(request({7,1}));
    SimReply too_many=accepted;too_many.reports.push_back(accepted.reports[0]);
    CHECK_EQ(link.capture(too_many),Error::Capacity);CHECK(link.reports().empty());
    SimReply mixed=accepted;mixed.reports={ReferenceMark{500}};
    CHECK_EQ(link.capture(mixed),Error::UntrustedEvidence);CHECK(link.reports().empty());
    mixed.reports={WorkingReport{header({7,1})}};
    CHECK_EQ(link.capture(mixed),Error::UntrustedEvidence);CHECK(link.reports().empty());
    CHECK_EQ(link.capture(accepted),Error::None);CHECK_EQ(link.reports()[0].id,1U);
    CHECK_EQ(link.capture(accepted),Error::Capacity);CHECK_EQ(link.reports().size(),1U);
}
TEST_CASE(DELIVERY_invalid_ids_and_attempt_capacity_do_not_touch_core){
    Fixture f(delivery_config());LocalSimVenue venue(example::simulator_config(delivery_config()));LocalReportDelivery link(2,1);
    const auto id=f.submit();CHECK_EQ(link.capture(venue.receive(request(id))),Error::None);
    CHECK_EQ(link.deliver(f.core,0,2).error,Error::InvalidInput);CHECK_EQ(link.deliver(f.core,2,2).error,Error::InvalidInput);
    CHECK(link.attempts().empty());CHECK_EQ(link.deliver(f.core,1,2).error,Error::None);
    const auto summary=f.core.canonical_state();
    CHECK_EQ(link.deliver(f.core,1,3).error,Error::Capacity);CHECK_EQ(f.core.canonical_state(),summary);
    CHECK_EQ(link.attempts().size(),1U);
}
