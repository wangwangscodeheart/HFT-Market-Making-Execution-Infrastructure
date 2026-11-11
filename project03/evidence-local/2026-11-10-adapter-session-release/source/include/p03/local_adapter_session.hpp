#pragma once
#include "p03/local_sim.hpp"
#include "p03/local_critical_inbox.hpp"
#include <future>
#include <thread>
namespace p03 {
struct LocalAdapterConfig {SimConfig peer;std::size_t commands{16},reports{32};std::uint64_t ttl_ms{5000};};
struct LocalDispatchSubmission {StepResult core;Error dispatch{Error::Blocked};std::optional<DispatchPermitId> permit;};
struct LocalAdapterOutcome {DispatchPermitId permit;OrderId order;Error error;bool called{};SimReply reply;};
// Two-context local experiment: caller owns CoreLoop, worker exclusively owns venue.
// Core acceptance -> copied command -> local-memory permit; not a durable SDK authorization.
class LocalAdapterSession {
public:
    LocalAdapterSession(CoreLoop&,LocalAdapterConfig);
    ~LocalAdapterSession();
    LocalAdapterSession(const LocalAdapterSession&)=delete;
    LocalAdapterSession& operator=(const LocalAdapterSession&)=delete;
    LocalDispatchSubmission submit(const SubmitOrder&,LogicalTime);
    void start();
    StepResult kill(LogicalTime);
    QueuePop<LocalAdapterOutcome> receive(); // Caller applies returned evidence on the core owner thread.
    const SimQueryResult& finish(); // Closes command input, drains, joins, and returns detached peer query.
    [[nodiscard]] LocalGateSnapshot gate_snapshot() const {return gate_.snapshot();}
private:
    struct Job {DispatchPermitId permit;SimOrderRequest request;LogicalTime at;std::chrono::steady_clock::time_point queued_at;};
    CoreLoop& core_;const std::thread::id owner_;const LocalAdapterConfig config_;
    LocalDispatchGate gate_;BoundedQueue<Job> commands_;LocalCriticalInbox<LocalAdapterOutcome> reports_;
    std::future<SimQueryResult> worker_;std::optional<SimQueryResult> final_;bool closed_{};
    void require_owner() const;
    SimQueryResult work();
};
}
