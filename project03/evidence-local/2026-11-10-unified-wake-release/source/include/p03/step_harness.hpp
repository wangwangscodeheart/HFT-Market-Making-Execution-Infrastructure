#pragma once
#include "p03/core.hpp"
#include <vector>

namespace p03 {
struct ScheduledStep { LogicalTime at{}; std::uint64_t insertion{}; BusinessEvent event; };
struct StepObservation { LogicalTime at{}; BusinessEvent input; StepResult result; std::string canonical_state; };
class StepHarness {
public:
    explicit StepHarness(CoreConfig config, std::size_t capacity = 1024)
        : core_(std::move(config)), capacity_(capacity) {}
    Error schedule(LogicalTime, BusinessEvent);
    Error run();
    [[nodiscard]] const CoreLoop& core() const noexcept { return core_; }
    [[nodiscard]] const std::vector<StepObservation>& trace() const noexcept { return trace_; }
private:
    CoreLoop core_;
    std::size_t capacity_;
    std::uint64_t insertion_{};
    LogicalTime time_{};
    std::vector<ScheduledStep> pending_;
    std::vector<StepObservation> trace_;
};
} // namespace p03
