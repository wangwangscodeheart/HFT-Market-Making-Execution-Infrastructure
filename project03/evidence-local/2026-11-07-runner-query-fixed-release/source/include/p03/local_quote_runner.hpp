#pragma once
#include "p03/local_sim.hpp"
#include "p03/quote_strategy.hpp"
namespace p03 {
struct MarketPulse {MarketTop book;SessionPoint session;};
struct LocalFillInput {OrderId order;std::string trade_id;Lots quantity{};Ticks price{};};
struct StopLocalQuotes {};
struct LocalQuoteTimer {};
struct LocalDeferredFillInput {LocalFillInput fill;};
struct LocalHeldReportInput {std::size_t index{};}; // Retained reports, zero-based; repeated delivery is allowed.
struct LocalTradeQueryInput {QueryId query;bool omit_last_trade{};}; // Explicit local fault seam.
using LocalQuoteInput=std::variant<MarketPulse,LocalFillInput,StopLocalQuotes,LocalQuoteTimer,
    LocalDeferredFillInput,LocalHeldReportInput,LocalTradeQueryInput>;
struct LocalCoreCall {BusinessEvent input;StepResult result;};
struct LocalOrderView {OrderId id;OrderIntent intent;QuantityParts parts;ExecutionState execution;};
struct LocalQuoteFrame {
    std::uint64_t ordinal{};
    LogicalTime at{};
    LocalQuoteInput input;
    Error error{Error::None};
    std::optional<QuoteDecision> decision;
    std::vector<LocalCoreCall> calls;
    std::string canonical;
    std::optional<SimQuerySnapshot> peer;
    bool completed{};
    CoreMode mode{CoreMode::Ready};
    std::array<Lots,4> positions{};
    std::vector<LocalOrderView> orders;
    std::size_t pending_reports{};
};
struct LocalQuoteRunnerConfig {
    CoreConfig core;
    SimConfig peer;
    QuoteStrategyConfig strategy;
    std::size_t max_inputs{128},max_core_calls{4096};
};
// Synchronous explicit-memory driver, no worker thread, clock, SDK or journal.
// External ordinal and core sequence remain distinct. Only this driver writes core.
class LocalQuoteRunner {
public:
    explicit LocalQuoteRunner(LocalQuoteRunnerConfig);
    Error step(LocalQuoteInput,LogicalTime);
    [[nodiscard]] const CoreLoop& core() const noexcept {return core_;}
    [[nodiscard]] const LocalSimVenue& peer() const noexcept {return peer_;}
    [[nodiscard]] const std::vector<LocalQuoteFrame>& trace() const noexcept {return trace_;}
    [[nodiscard]] bool stopped() const noexcept {return stopped_;}
private:
    CoreLoop core_;LocalSimVenue peer_;QuoteStrategy strategy_;
    SimConfig peer_config_;
    std::vector<TradeReport> held_;
    std::size_t input_capacity_,call_capacity_,call_count_{},per_input_bound_;
    std::uint64_t generation_{};LogicalTime last_time_{};bool stopped_{};
    std::optional<LogicalTime> last_market_at_;
    LogicalTime market_max_age_{};
    std::vector<LocalQuoteFrame> trace_;
    StepResult call(LocalQuoteFrame&,const BusinessEvent&);
    Error reports(LocalQuoteFrame&,const SimReply&);
    Error cancel(LocalQuoteFrame&,OrderId);
    Error withdraw(LocalQuoteFrame&);
    Error market(LocalQuoteFrame&,const MarketPulse&);
    [[nodiscard]] std::size_t pending_reports() const;
};
} // namespace p03
