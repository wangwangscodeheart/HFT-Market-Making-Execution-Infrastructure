#pragma once
#include "p03/core.hpp"
namespace p03 {
struct QuoteLevel {Ticks price{};Lots quantity{1};};
struct QuoteTarget {
    BusinessScope scope;
    StrategyId strategy;
    std::uint64_t generation{};
    SessionPoint session;
    std::optional<QuoteLevel> buy,sell;
};
using PlannedIntent=std::variant<SubmitOrder,RequestCancel>;
struct QuotePlan {Error error{Error::None};std::uint64_t generation{};std::vector<PlannedIntent> intents;};
// Read-only planning; caller must replan after cancellations and submit through
// CoreLoop risk admission. No production command, reservation, or gateway effect.
[[nodiscard]] QuotePlan plan_quotes(const CoreLoop&,const QuoteTarget&,std::uint64_t current_generation);
} // namespace p03
