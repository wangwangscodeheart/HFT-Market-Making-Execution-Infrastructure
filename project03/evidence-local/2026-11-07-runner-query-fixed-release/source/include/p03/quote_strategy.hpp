#pragma once
#include "p03/order_planner.hpp"
namespace p03 {
struct MarketTop {
    BusinessScope scope;
    EvidenceOrigin origin{EvidenceOrigin::Unverified};
    std::string source;
    std::uint64_t epoch{},sequence{};
    LogicalTime received_at{};
    Ticks bid{},ask{};
    Lots bid_quantity{},ask_quantity{};
    bool healthy{};
};
struct QuoteStrategyConfig {
    BusinessScope scope;
    StrategyId strategy;
    EvidenceOrigin origin{EvidenceOrigin::LocalFixture};
    std::string source;
    std::uint64_t market_epoch{1};
    LogicalTime max_age{10};
    Lots quantity{1},inventory_limit{10};
};
struct QuoteDecision {
    Error error{Error::None};
    bool withdraw{}; // Request to withdraw desired quotes, not proof of cancellation.
    std::optional<QuoteTarget> target;
};
// Minimal deterministic target generator. No predictive model or return claims.
class QuoteStrategy {
public:
    explicit QuoteStrategy(QuoteStrategyConfig);
    QuoteDecision evaluate(const CoreLoop&,const MarketTop&,SessionPoint,LogicalTime now,std::uint64_t generation);
private:
    QuoteStrategyConfig config_;
    std::uint64_t last_sequence_{},last_generation_{};
};
} // namespace p03
