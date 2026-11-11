#pragma once
#include "p03/local_adapter_session.hpp"
#include "p03/quote_strategy.hpp"
namespace p03 {
struct RuntimeStart {};
struct RuntimeCancel {OrderId order;};
struct RuntimeFill {OrderId order;std::string trade_id;Lots quantity{};Ticks price{};};
struct RuntimePump {std::size_t budget{8};};
struct RuntimeRetry {};
struct RuntimeFinish {};
struct RuntimeMarketSnapshot {MarketSnapshot snapshot;SessionPoint session;};
struct RuntimeFeedDisconnect {};
struct RuntimeFeedSubscription {std::uint64_t generation{};};
struct RuntimeQuoteTimer {SessionPoint session;};
using LocalRuntimeInput=std::variant<RuntimeStart,ReferenceMark,SubmitOrder,RuntimeCancel,RuntimeFill,KillRequested,RuntimePump,RuntimeRetry,RuntimeFinish,
    RuntimeMarketSnapshot,RuntimeFeedDisconnect,RuntimeFeedSubscription,RuntimeQuoteTimer>;
struct LocalRuntimeFrame {
    std::size_t ordinal{};LogicalTime at{};LocalRuntimeInput input;Error error{Error::None};
    std::optional<StepResult> core_result;std::optional<LocalPumpResult> pump_result;
    LocalGateSnapshot gate;std::string canonical;bool cleanup{},completed{};
    std::optional<FeedHealth> feed;std::optional<QuoteDecision> decision;std::optional<QuotePlan> plan;
};
struct RuntimeQuoteConfig {MarketStateConfig market;QuoteStrategyConfig strategy;};
struct LocalRuntimeConfig {CoreConfig core;LocalAdapterConfig adapter;std::size_t inputs{128},cleanup_inputs{16};std::optional<RuntimeQuoteConfig> quote{};};
// Owns core before session, destroys session (including join) before core.
// Two execution contexts only; no journal Writer, BridgeIO or production SDK.
class LocalRuntimeRunner {
public:
    explicit LocalRuntimeRunner(LocalRuntimeConfig);
    Error step(LocalRuntimeInput,LogicalTime);
    [[nodiscard]] const CoreLoop& core() const {require_owner();return core_;}
    [[nodiscard]] const std::vector<LocalRuntimeFrame>& trace() const {require_owner();return trace_;}
    [[nodiscard]] std::size_t pending_outcomes() const {require_owner();return session_.pending_outcomes();}
    [[nodiscard]] const std::optional<SimQueryResult>& peer_query() const {require_owner();return peer_query_;}
    [[nodiscard]] bool stopped() const {require_owner();return stopped_;}
private:
    const std::thread::id owner_;CoreLoop core_;LocalAdapterSession session_;
    std::size_t normal_capacity_,cleanup_capacity_,normal_count_{},cleanup_count_{};
    bool started_{},finished_{},stopped_{},fatal_{};LogicalTime last_time_{};
    std::vector<LocalRuntimeFrame> trace_;std::optional<SimQueryResult> peer_query_;
    std::optional<MarketState> market_;std::optional<QuoteStrategy> strategy_;std::optional<SessionPoint> quote_session_;std::uint64_t quote_generation_{};
    void require_owner() const;
    Error withdraw_quotes();
    Error drive_quotes(LocalRuntimeFrame&,LogicalTime);
};
}
