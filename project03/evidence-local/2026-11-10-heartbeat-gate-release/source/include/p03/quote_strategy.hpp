#pragma once
#include "p03/order_planner.hpp"
#include "p03/market_state.hpp"
namespace p03 {
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
    QuoteDecision evaluate(const CoreLoop&,const MarketState&,SessionPoint,LogicalTime now,std::uint64_t generation);
    // Recompute inventory targets from the currently qualified snapshot; does not refresh its receive age.
    QuoteDecision refresh(const CoreLoop&,const MarketState&,SessionPoint,LogicalTime now,std::uint64_t generation) const;
private:
    QuoteStrategyConfig config_;
    std::uint64_t last_sequence_{},last_generation_{},last_subscription_generation_{};
};
} // namespace p03
