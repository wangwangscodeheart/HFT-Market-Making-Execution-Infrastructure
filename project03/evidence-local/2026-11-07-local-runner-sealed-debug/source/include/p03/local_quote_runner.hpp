#pragma once
#include "p03/local_sim.hpp"
#include "p03/quote_strategy.hpp"
namespace p03 {
struct MarketPulse {MarketTop book;SessionPoint session;};
struct LocalFillInput {OrderId order;std::string trade_id;Lots quantity{};Ticks price{};};
struct StopLocalQuotes {};
using LocalQuoteInput=std::variant<MarketPulse,LocalFillInput,StopLocalQuotes>;
struct LocalCoreCall {BusinessEvent input;StepResult result;};
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
    std::size_t input_capacity_,call_capacity_,call_count_{},per_input_bound_;
    std::uint64_t generation_{};LogicalTime last_time_{};bool stopped_{};
    std::vector<LocalQuoteFrame> trace_;
    StepResult call(LocalQuoteFrame&,const BusinessEvent&);
    Error reports(LocalQuoteFrame&,const SimReply&);
    Error cancel(LocalQuoteFrame&,OrderId);
    Error withdraw(LocalQuoteFrame&);
    Error market(LocalQuoteFrame&,const MarketPulse&);
};
} // namespace p03
