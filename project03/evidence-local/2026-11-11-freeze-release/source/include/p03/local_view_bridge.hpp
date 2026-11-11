#pragma once
#include "p03/core.hpp"
#include "p03/local_dispatch_gate.hpp"
#include "p03/bounded_queue.hpp"
#include "p03/work_signal.hpp"
#include <atomic>
#include <future>
namespace p03 {
struct BridgeOrderView {OrderId id;Side side{Side::Buy};Offset offset{Offset::Open};Ticks price{};Lots quantity{};QuantityParts parts;ExecutionState execution{ExecutionState::Created};};
struct LocalBridgeView {
    std::uint64_t snapshot{},instance{};LogicalTime at{};BusinessScope scope;CoreMode mode{CoreMode::Ready};
    std::array<Lots,4> positions{};Money required{};RuntimeFaultState fault;LocalGateSnapshot gate;
    std::array<BridgeOrderView,64> orders{};std::size_t order_count{};
};
struct LocalBridgeConfig {std::size_t inputs{8},outputs{8};};
struct LocalBridgeStats {std::uint64_t accepted{},encoded{},dropped_inputs{},dropped_outputs{},encoding_errors{};bool closed{},failed{};};
// Optional local BridgeIO context. Owns detached views and JSON, never CoreLoop references.
// Lossy observation channel, not order evidence, a journal or a network service.
class LocalViewBridge {
public:
    explicit LocalViewBridge(LocalBridgeConfig);
    ~LocalViewBridge();
    bool publish(LocalBridgeView);
    QueuePop<std::string> receive(){return outputs_.try_pop();}
    void note_publish_failure() noexcept {increment(dropped_inputs_);}
    void close();
    [[nodiscard]] LocalBridgeStats stats() const;
private:
    BoundedQueue<LocalBridgeView> inputs_;BoundedQueue<std::string> outputs_;WorkSignal wake_;std::mutex join_mutex_;
    std::atomic<std::uint64_t> accepted_{},encoded_{},dropped_inputs_{},dropped_outputs_{},encoding_errors_{};
    std::atomic<bool> closed_{},failed_{};std::future<void> worker_;
    static void increment(std::atomic<std::uint64_t>&) noexcept;
    static bool valid(const LocalBridgeView&);
    static std::string encode(const LocalBridgeView&);
    void work();
};
}
