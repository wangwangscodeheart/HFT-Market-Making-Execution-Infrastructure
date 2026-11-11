#pragma once
#include "p03/types.hpp"
#include <array>
#include <vector>

namespace p03 {
struct PositionAnchor {
    std::string account;
    std::string instrument;
    TradingDay day;
    std::uint64_t epoch{};
    std::string trade_coverage; // Explicit initial coverage boundary, not an external certificate.
    std::array<Lots, 4> booked{};
    bool operator==(const PositionAnchor&) const = default;
};
struct ExternalPositionObservation {
    TradingDay day;
    std::uint64_t observation_id{};
    std::array<Lots, 4> reported{};
    std::string account, instrument;
    EvidenceOrigin origin{EvidenceOrigin::Unverified};
    ObservationMetadata metadata;
    std::string shape; // Four buckets are only a fixture schema; real CTP mapping is blocked.
};
struct SettlementObservation {
    TradingDay day;
    std::uint64_t record_id{};
    Lots reported_total{}; // Fixture statement quantity; not an equity/settlement calculation.
    std::string account;
    EvidenceOrigin origin{EvidenceOrigin::Unverified};
    ObservationMetadata metadata;
};
class PositionLedger {
public:
    explicit PositionLedger(PositionAnchor anchor);
    [[nodiscard]] const PositionAnchor& anchor() const noexcept { return anchor_; }
    [[nodiscard]] const std::array<Lots,4>& booked() const noexcept { return booked_; }
    [[nodiscard]] const std::array<Lots,4>& frozen() const noexcept { return frozen_; }
    [[nodiscard]] Lots net() const;
    [[nodiscard]] Lots gross() const;
    [[nodiscard]] Lots available(Bucket b) const;
    [[nodiscard]] bool deficit() const noexcept;
    void replace_freezes(const std::array<Lots,4>&);
    void book(Side, Offset, Lots quantity); // Valid economic facts may expose a negative bucket.
    [[nodiscard]] static Bucket affected_bucket(Side, Offset) noexcept;
private:
    PositionAnchor anchor_;
    std::array<Lots,4> booked_{};
    std::array<Lots,4> frozen_{};
};
} // namespace p03
