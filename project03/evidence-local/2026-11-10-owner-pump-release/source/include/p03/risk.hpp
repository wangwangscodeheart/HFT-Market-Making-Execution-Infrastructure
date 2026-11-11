#pragma once
#include "p03/funds.hpp"
#include "p03/ledger.hpp"
#include "p03/oms.hpp"
#include "p03/positions.hpp"

namespace p03 {
struct RiskLimits {
    Lots max_order{10}, max_abs_net{100}, max_gross{200};
    Money local_funds{1'000'000}, safety_buffer{0};
    Money opening_margin_per_lot{100}, fee_per_lot{2}, anchor_margin{};
    Ticks max_deviation{100};
    LogicalTime max_mark_age{100};
    std::size_t order_rate{100};
    LogicalTime rate_window{100};
};
struct RiskView {
    Lots booked_net{}, min_net{}, max_net{}, gross_bound{}, exception_lots{};
    bool unknown{}, position_deficit{}, funds_deficit{};
};
class RiskEngine {
public:
    explicit RiskEngine(RiskLimits);
    [[nodiscard]] const RiskLimits& limits() const noexcept { return limits_; }
    [[nodiscard]] RiskView calculate(const PositionLedger&, const std::map<OrderId,Order>&,
        const ExecutionLedger&, bool unknown) const;
    [[nodiscard]] std::vector<FundResource> resources(const PositionLedger&,
        const std::map<OrderId,Order>&, const ExecutionLedger&, ReservationId anchor_resource) const;
    [[nodiscard]] Error check(const RiskView&, const FundsView&) const noexcept;
    [[nodiscard]] bool crosses(const OrderIntent&, const std::map<OrderId,Order>&) const;
private:
    RiskLimits limits_;
};
} // namespace p03
