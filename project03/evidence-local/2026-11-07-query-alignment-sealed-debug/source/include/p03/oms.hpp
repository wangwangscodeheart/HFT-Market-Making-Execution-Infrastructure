#pragma once
#include "p03/types.hpp"
#include <map>
#include <tuple>

namespace p03 {
enum class ExecutionState { Created, Working, PartiallyFilled, Filled, Canceled, Rejected, NotSent };
enum class SendState { NotImplemented }; // There is deliberately no send eligibility in M1A.
enum class CancelState { None, Requested, Succeeded, Rejected, Unknown };
enum class Consistency { InSync, MissingTrades, Unreconciled, Conflict };
enum class TerminalKind { Filled, Canceled, Rejected, NotSent };
struct TerminalProof { TerminalKind kind; std::optional<Lots> exact_quantity; };
class Order {
public:
    Order(OrderId, ReservationId, OrderIntent, std::size_t cancel_capacity=8,
        std::optional<ClientOrderId> client_id={}, StrategyId strategy={"fixture.default.strategy"});
    [[nodiscard]] OrderId id() const noexcept { return id_; }
    [[nodiscard]] ReservationId reservation() const noexcept { return reservation_; }
    [[nodiscard]] const OrderIntent& intent() const noexcept { return intent_; }
    [[nodiscard]] const std::optional<ClientOrderId>& client_id() const noexcept { return client_id_; }
    [[nodiscard]] const StrategyId& strategy() const noexcept { return strategy_; }
    [[nodiscard]] Lots booked() const noexcept { return booked_; }
    [[nodiscard]] Lots lower_bound() const noexcept { return lower_bound_; }
    [[nodiscard]] QuantityParts parts() const;
    [[nodiscard]] ExecutionState execution() const noexcept { return execution_; }
    [[nodiscard]] SendState send() const noexcept { return SendState::NotImplemented; }
    [[nodiscard]] CancelState cancel() const noexcept { return cancel_; }
    [[nodiscard]] Consistency consistency() const;
    [[nodiscard]] const std::optional<ExternalOrderIdentity>& external_identity() const noexcept { return external_; }
    [[nodiscard]] std::optional<CancelActionId> cancel_action() const noexcept { return action_; }
    [[nodiscard]] bool invariant() const;
    [[nodiscard]] std::size_t stream_count() const noexcept { return versions_.size(); }
    [[nodiscard]] bool has_stream(const EvidenceStamp& s) const { return versions_.contains({s.source,s.epoch}); }
    [[nodiscard]] std::size_t cancel_action_count() const noexcept { return actions_.size(); }
    [[nodiscard]] bool has_cancel_action(CancelActionId id) const { return actions_.contains(id); }
    [[nodiscard]] std::string canonical_state() const;

    Error observe_cumulative(Lots, const EvidenceStamp&);
    Error observe_terminal(TerminalProof);
    Error observe_working() noexcept;
    void book_trade(Lots); // Call only after the execution ledger has deduplicated the trade.
    void mark_conflict() noexcept { conflict_ = true; }
    void mark_unknown() noexcept { unknown_ = true; }
    Error request_cancel(CancelActionId);
    Error observe_cancel(CancelActionId, CancelState);
private:
    struct Version { std::uint64_t seq{}; Lots quantity{}; };
    OrderId id_;
    ReservationId reservation_;
    OrderIntent intent_;
    std::optional<ClientOrderId> client_id_;
    StrategyId strategy_;
    Lots booked_{}, lower_bound_{};
    std::optional<Lots> final_;
    std::optional<TerminalKind> terminal_kind_;
    bool no_future_{}, conflict_{}, unknown_{};
    bool accepted_{};
    ExecutionState execution_{ExecutionState::Created};
    CancelState cancel_{CancelState::None};
    std::optional<CancelActionId> action_;
    std::optional<ExternalOrderIdentity> external_;
    std::size_t cancel_capacity_;
    std::map<CancelActionId,CancelState> actions_;
    std::map<std::pair<std::string,std::uint64_t>, Version> versions_;
};
} // namespace p03
