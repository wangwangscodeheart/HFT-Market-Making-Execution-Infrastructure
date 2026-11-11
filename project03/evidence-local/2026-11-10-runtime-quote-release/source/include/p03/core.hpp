#pragma once
#include "p03/reference_data.hpp"
#include "p03/risk.hpp"
#include <variant>
#include <tuple>
#include <atomic>

namespace p03 {
enum class CoreMode { Ready, Killed, ConflictHalted, BoundaryHalted, AwaitDayEvidence, CapacityHalted };
struct CapacityLimits {
    std::size_t orders{64}, trades{256}, quarantine{32}, observations{16}, evidence_streams_per_order{8}, cancel_actions_per_order{8};
};
struct CoreConfig {
    std::uint64_t instance{1};
    std::uint64_t next_order{1};
    Contract contract;
    TradingCalendar calendar;
    PositionAnchor anchor;
    RiskLimits risk;
    CapacityLimits capacity;
    StrategyId strategy{"fixture.default.strategy"};
    TradeEvidenceMode trade_evidence_mode{TradeEvidenceMode::Fixture};
};
struct SubmitOrder {
    OrderIntent intent;
    SessionPoint session;
    std::optional<ClientOrderId> client_id{};
    std::optional<StrategyId> strategy{}; // Absent means this explicitly configured single-strategy context.
};
struct ReferenceMark { Ticks price{}; };
struct EvidenceHeader { OrderId order; std::string account; TradingDay day; EvidenceStamp stamp; };
struct WorkingReport { EvidenceHeader header; };
struct CumulativeReport { EvidenceHeader header; Lots quantity{}; };
struct TerminalReport { EvidenceHeader header; TerminalProof proof; };
struct TradeReport { Trade trade; EvidenceStamp stamp; Channel channel{Channel::Push}; };
struct RequestCancel { OrderId order; };
struct CancelReport { EvidenceHeader header; CancelActionId action; CancelState state; };
struct OrderTimeout { OrderId order; };
struct KillRequested {};
struct RuntimeFaultState {
    std::uint32_t reasons{};std::uint64_t latch_version{},loss_generation{};
    bool operator==(const RuntimeFaultState&) const = default;
};
// Local adapter latch snapshot; records stops, never grants send/recovery eligibility.
struct LocalRuntimeFault {BusinessScope scope;std::uint64_t instance{};RuntimeFaultState fault;};
struct DayBoundary { TradingDay next_day; };
struct ObservePosition { ExternalPositionObservation observation; };
struct ObserveSettlement { SettlementObservation observation; };
struct ObserveFunds { FundsObservation observation; };
enum class FutureOperation { SendOrder, EmergencyCancel, JournalAppend, ReconcileExternal, CtpConnect,
    LocalSimulator, WebBridge, ThreadRuntime, RestoreSnapshot, Reactivate };
struct RequestFutureOperation { FutureOperation operation; };
using BusinessEvent = std::variant<SubmitOrder,ReferenceMark,WorkingReport,CumulativeReport,TerminalReport,
    TradeReport,RequestCancel,CancelReport,OrderTimeout,KillRequested,DayBoundary,ObservePosition,
    ObserveSettlement,ObserveFunds,RequestFutureOperation,LocalRuntimeFault>;

struct QuarantinedEvidence {
    Error reason;
    OrderId order;
    Lots reported_quantity{};
    std::optional<Trade> trade; // Bounded valid payload; no silent trade ledger overwrite.
    std::uint64_t ingress_sequence{}, core_sequence{}; // Original attempt -> ordered local quarantine fact.
};
enum class InputAdmission { RejectedInput, BusinessInput, RejectedEvidenceFact };
[[nodiscard]] std::string_view admission_name(InputAdmission) noexcept;
struct StepResult {
    Error error{Error::None};
    bool applied{};
    CommandId command;
    std::uint64_t core_sequence{};
    std::uint64_t ingress_sequence{}; // Local call attempts, including rejected inputs; not an exchange sequence.
    InputAdmission admission{InputAdmission::RejectedInput};
    std::optional<OrderId> order;
    std::optional<CancelActionId> cancel;
};
struct CoreState {
    friend void swap(CoreState& a, CoreState& b) noexcept {
        auto left = std::tie(a.orders, a.client_orders, a.positions, a.executions, a.funds, a.quarantine,
            a.external_positions, a.settlements, a.order_times, a.mark, a.proposed_day,
            a.mode, a.unknown_exposure, a.risk_deficit, a.evidence_loss_count,a.runtime_fault);
        auto right = std::tie(b.orders, b.client_orders, b.positions, b.executions, b.funds, b.quarantine,
            b.external_positions, b.settlements, b.order_times, b.mark, b.proposed_day,
            b.mode, b.unknown_exposure, b.risk_deficit, b.evidence_loss_count,b.runtime_fault);
        static_assert(noexcept(left.swap(right)));
        left.swap(right);
    }
    std::map<OrderId,Order> orders;
    std::map<ClientRequestKey,OrderId> client_orders; // Accepted requests only; scoped to this CoreLoop.
    PositionLedger positions;
    ExecutionLedger executions;
    FundsLedger funds;
    std::vector<QuarantinedEvidence> quarantine;
    std::vector<ExternalPositionObservation> external_positions;
    std::vector<SettlementObservation> settlements;
    std::vector<LogicalTime> order_times;
    std::optional<std::pair<Ticks,LogicalTime>> mark;
    std::optional<TradingDay> proposed_day;
    CoreMode mode{CoreMode::Ready};
    bool unknown_exposure{}, risk_deficit{};
    std::uint64_t evidence_loss_count{};
    RuntimeFaultState runtime_fault;
    explicit CoreState(const CoreConfig& c) : positions(c.anchor), funds(c.risk.local_funds,c.risk.safety_buffer,
        c.anchor.account,c.anchor.day,c.anchor.epoch) {}
};

// Single owner, single synchronous step. No API exposes a mutable CoreState or an outbound effect.
// M1A copy/validate/swap transactions deliberately prioritize exception safety over throughput.
class CoreLoop {
    friend class LocalAdapterSession;
public:
    explicit CoreLoop(CoreConfig);
    CoreLoop(const CoreLoop&) = delete;
    CoreLoop& operator=(const CoreLoop&) = delete;
    StepResult step(const BusinessEvent&, LogicalTime);
    [[nodiscard]] const CoreState& state() const noexcept { return state_; }
    [[nodiscard]] const CoreConfig& config() const noexcept { return config_; }
    [[nodiscard]] const BusinessScope& scope() const noexcept { return scope_; }
    [[nodiscard]] RiskView risk_view() const;
    [[nodiscard]] bool invariant() const;
    [[nodiscard]] std::string canonical_state() const;
    [[nodiscard]] static Availability capability(FutureOperation) noexcept;
    // Explicit deterministic exception-safety seam, no production fault runtime or threads.
    void fail_next_commit_for_test() noexcept { fail_next_commit_ = true; }
private:
    std::atomic<bool> local_adapter_attached_{false}; // Lifetime lease, not business state or send eligibility.
    CoreConfig config_;
    const BusinessScope scope_;
    RiskEngine risk_;
    CoreState state_;
    IdSequence<OrderTag> order_ids_;
    IdSequence<CommandTag> command_ids_;
    IdSequence<CancelTag> cancel_ids_;
    IdSequence<ReservationTag> reservation_ids_;
    ReservationId anchor_resource_;
    std::uint64_t sequence_{};
    std::uint64_t ingress_sequence_{};
    LogicalTime last_time_{};
    bool fail_next_commit_{};

    struct InputCheck {
        Error error{Error::None};
        bool critical{};
        OrderId order{};
        Lots quantity{};
        OrderId related_order{};
    };
    [[nodiscard]] InputCheck validate_input(const BusinessEvent&) const;

    Error apply(CoreState&, const BusinessEvent&, LogicalTime, StepResult&,
        IdSequence<OrderTag>&, IdSequence<CancelTag>&, IdSequence<ReservationTag>&);
    Error header_error(const CoreState&, const EvidenceHeader&) const;
    void quarantine(CoreState&, Error, OrderId, Lots, std::optional<Trade> = {});
    void recompute(CoreState&) const;
    [[nodiscard]] bool invariant(const CoreState&) const;
};
} // namespace p03
