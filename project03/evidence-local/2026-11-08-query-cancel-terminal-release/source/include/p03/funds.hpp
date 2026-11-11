#pragma once
#include "p03/types.hpp"
#include <map>
#include <vector>
#include <tuple>

namespace p03 {
struct FundResource {
    ReservationId id;
    Money order_margin{}, position_margin{}, charges{};
    [[nodiscard]] Money margin() const { return checked_add(order_margin,position_margin); }
    [[nodiscard]] Money total() const { return checked_add(margin(),charges); }
    bool operator==(const FundResource&) const = default;
};
enum class CoverageStatus { Proven, Unknown, Excluded };
struct CoverageProof {
    ReservationId resource;
    CoverageStatus status;
    Money amount{};
    std::string external_deduction_id{};
    bool operator==(const CoverageProof&) const = default;
};
struct FundsObservation {
    std::string account;
    TradingDay day;
    std::uint64_t position_epoch{};
    std::uint64_t snapshot_id{};
    Money external_available{};
    EvidenceOrigin origin{EvidenceOrigin::Unverified};
    std::vector<CoverageProof> coverage;
    ObservationMetadata metadata;
    bool operator==(const FundsObservation&) const = default;
};
struct FundsView {
    Money local_required{}, proven_cover{}, uncovered{}, external_budget{};
    bool has_external{}, local_ok{}, external_ok{};
    [[nodiscard]] bool sufficient() const noexcept { return local_ok && external_ok; }
};
class FundsLedger {
public:
    friend void swap(FundsLedger& a, FundsLedger& b) noexcept {
        auto left = std::tie(a.local_cap_, a.safety_buffer_, a.account_, a.day_,
            a.position_epoch_, a.resources_, a.observation_);
        auto right = std::tie(b.local_cap_, b.safety_buffer_, b.account_, b.day_,
            b.position_epoch_, b.resources_, b.observation_);
        static_assert(noexcept(left.swap(right)));
        left.swap(right);
    }
    FundsLedger(Money local_cap, Money safety_buffer, std::string account, TradingDay, std::uint64_t position_epoch);
    void replace_resources(std::vector<FundResource>); // Resource IDs retain ownership across transfer.
    Error observe(FundsObservation); // Arithmetic fixture only; no CTP coverage profile is enabled.
    [[nodiscard]] Error validate(const FundsObservation&) const;
    [[nodiscard]] FundsView view() const;
    [[nodiscard]] const std::map<ReservationId,FundResource>& resources() const noexcept { return resources_; }
    [[nodiscard]] const std::optional<FundsObservation>& observation() const noexcept { return observation_; }
private:
    Money local_cap_{}, safety_buffer_{};
    std::string account_;
    TradingDay day_;
    std::uint64_t position_epoch_{};
    std::map<ReservationId,FundResource> resources_;
    std::optional<FundsObservation> observation_;
};
} // namespace p03
