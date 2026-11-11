#pragma once
#include "p03/core.hpp"
#include "p03/recovery_facts.hpp"

namespace p03 {
struct SimRules {
    std::string instrument,exchange;
    TradingDay day;
    Ticks low{},high{},tick_step{1};
    Lots min_lots{1},max_lots{10},lot_step{1};
};
struct SimConfig {
    std::string account;
    SimRules rules;
    std::array<Lots,4> initial_positions{};
    Money initial_budget{1'000'000},opening_margin_per_lot{100},fee_per_lot{2};
    std::uint64_t model_epoch{1};
    std::size_t max_orders{64},max_trades{256},max_cancels{256};
};
// Peer-side message value, not a production authorization token or SDK command.
struct SimOrderRequest {
    OrderId order;
    OrderIntent intent;
    StrategyId strategy;
    std::optional<ClientOrderId> client_id{};
    bool operator==(const SimOrderRequest&) const = default;
};
enum class SimOrderStatus { Working,PartiallyFilled,Filled,Canceled,Rejected };
struct SimOrderRecord {
    SimOrderRequest request;
    std::string venue_reference; // Explicit simulated venue reference; never CTP OrderSysID.
    SimOrderStatus status{SimOrderStatus::Working};
    Lots filled{};
    Money reserved_margin{},reserved_fees{},position_margin{},paid_fees{};
};
struct SimReply {
    Error error{Error::None};
    std::uint64_t revision{};
    std::vector<BusinessEvent> reports; // Caller controls delivery; simulator never calls the core.
};
struct SimQuerySnapshot {
    QueryId query;
    EvidenceOrigin origin{EvidenceOrigin::LocalSimulator};
    std::uint64_t model_epoch{},revision{};
    std::string account,instrument;
    TradingDay day;
    std::array<Lots,4> positions{},frozen{};
    Money available_budget{};
    std::vector<SimOrderRecord> orders;
    std::vector<TradeReport> trades; // Query channel, same economic trade IDs as push.
    std::size_t expected_orders{},expected_trades{};
    bool complete{}; // Local synchronous snapshot boundary, not a recovery certificate.
};
struct SimQueryResult { Error error{Error::None};std::optional<SimQuerySnapshot> snapshot; };

class LocalSimVenue {
public:
    explicit LocalSimVenue(SimConfig);
    LocalSimVenue(const LocalSimVenue&)=delete;
    LocalSimVenue& operator=(const LocalSimVenue&)=delete;
    SimReply receive(const SimOrderRequest&);
    SimReply execute_fill(OrderId,std::string trade_id,Lots,Ticks);
    SimReply cancel(OrderId,CancelActionId);
    [[nodiscard]] SimQueryResult query(QueryId) const;
    [[nodiscard]] bool invariant() const;
private:
    struct CancelRecord { OrderId order;std::vector<BusinessEvent> reports; };
    struct State {
        std::map<OrderId,SimOrderRecord> orders;
        std::map<std::string,TradeReport> trades;
        std::map<CancelActionId,CancelRecord> cancels;
        std::array<Lots,4> positions{},frozen{};
        Money available{};
        std::uint64_t revision{};
        friend void swap(State& a,State& b) noexcept {
            auto left=std::tie(a.orders,a.trades,a.cancels,a.positions,a.frozen,a.available,a.revision);
            auto right=std::tie(b.orders,b.trades,b.cancels,b.positions,b.frozen,b.available,b.revision);
            static_assert(noexcept(left.swap(right)));left.swap(right);
        }
    };
    SimConfig config_;
    Money initial_margin_{};
    State state_;
    [[nodiscard]] Error request_error(const SimOrderRequest&) const;
    [[nodiscard]] bool invariant(const State&) const;
    [[nodiscard]] EvidenceStamp stamp(std::uint64_t revision,std::string source) const;
    [[nodiscard]] static std::size_t bucket(Side,Offset);
    [[nodiscard]] static bool active(SimOrderStatus) noexcept;
    void commit(State&);
};
} // namespace p03
