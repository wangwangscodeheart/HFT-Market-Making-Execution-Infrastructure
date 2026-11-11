#pragma once
#include "p03/reference_data.hpp"
#include <array>
namespace p03 {
struct MarketTop {
    BusinessScope scope;EvidenceOrigin origin{EvidenceOrigin::Unverified};std::string source;
    std::uint64_t epoch{},sequence{};LogicalTime received_at{};
    Ticks bid{},ask{};Lots bid_quantity{},ask_quantity{};bool healthy{};
    std::uint64_t subscription_generation{1};
};
constexpr std::size_t max_market_depth=5;
constexpr std::uint32_t market_required_fields=15; // bid/ask prices and quantities
struct MarketLevel {Ticks price{};Lots quantity{};bool operator==(const MarketLevel&) const = default;};
struct MarketSnapshot {
    BusinessScope scope;EvidenceOrigin origin{EvidenceOrigin::Unverified};std::string source;
    std::uint64_t source_epoch{},subscription_generation{},receive_sequence{};
    LogicalTime received_at{};std::optional<std::uint64_t> source_time;
    std::size_t valid_depth{};std::uint32_t valid_fields{};
    std::array<MarketLevel,max_market_depth> bids{},asks{};
    bool operator==(const MarketSnapshot&) const = default;
};
enum class FeedStatus {AwaitingData,Valid,Stale,Invalid,Recovering};
enum class FeedReason {AwaitingSnapshot,None,Expired,MissingFields,MissingDepth,InvalidBook,SequenceConflict,
    FutureTimestamp,ReceiveTimeRegression,Disconnected,LocalLoss,AwaitingFreshSnapshot,TimeRegression,ResourceFailure,TradingDayMismatch};
struct FeedHealth {
    bool operator==(const FeedHealth&) const = default;
    FeedStatus status;FeedReason reason;std::uint64_t subscription_generation{},last_receive_sequence{};
    std::optional<LogicalTime> last_valid_received_at;
};
struct MarketStateConfig {
    BusinessScope scope;Contract contract;EvidenceOrigin origin{EvidenceOrigin::LocalFixture};std::string source;
    std::uint64_t source_epoch{1},initial_generation{1};std::size_t required_depth{1};LogicalTime max_age{10};
};
// Fixed-profile full snapshots. receive_sequence is an adapter-local counter,
// not an exchange sequence; reconnect requires an explicit newer subscription.
class MarketState {
public:
    explicit MarketState(MarketStateConfig);
    Error update(const MarketSnapshot&,LogicalTime now);
    Error tick(LogicalTime now);
    Error disconnect(LogicalTime now);
    Error local_loss(LogicalTime now);
    Error begin_subscription(std::uint64_t generation,LogicalTime now);
    [[nodiscard]] FeedHealth health(LogicalTime now) const;
    [[nodiscard]] std::optional<MarketTop> top(LogicalTime now) const;
    [[nodiscard]] const std::optional<MarketSnapshot>& snapshot() const noexcept {return snapshot_;}
private:
    MarketStateConfig config_;FeedStatus status_{FeedStatus::AwaitingData};FeedReason reason_{FeedReason::AwaitingSnapshot};
    std::uint64_t generation_{},last_sequence_{};LogicalTime last_time_{};bool requires_new_generation_{};
    bool day_boundary_halted_{};
    std::optional<LogicalTime> last_received_;std::optional<MarketSnapshot> snapshot_;
    Error touch(LogicalTime);
    Error invalid(Error,FeedReason);
};
}
