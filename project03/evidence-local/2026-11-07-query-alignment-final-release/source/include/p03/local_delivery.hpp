#pragma once
#include "p03/local_sim.hpp"

namespace p03 {
struct CapturedSimReport {
    std::uint64_t id{},peer_revision{};
    BusinessEvent event;
};
struct LocalDeliveryAttempt {
    std::uint64_t report_id{};
    LogicalTime at{};
    StepResult result;
};
// Bounded, synchronous experimental transport. Retained reports may be delivered
// out of order or repeatedly; IDs are local capture IDs, never venue sequences.
class LocalReportDelivery {
public:
    explicit LocalReportDelivery(std::size_t reports=256,std::size_t attempts=512);
    Error capture(const SimReply&);
    StepResult deliver(CoreLoop&,std::uint64_t report_id,LogicalTime);
    [[nodiscard]] const std::vector<CapturedSimReport>& reports() const noexcept {return reports_;}
    [[nodiscard]] const std::vector<LocalDeliveryAttempt>& attempts() const noexcept {return attempts_;}
private:
    std::size_t report_capacity_,attempt_capacity_;
    std::vector<CapturedSimReport> reports_;
    std::vector<LocalDeliveryAttempt> attempts_;
};
} // namespace p03
