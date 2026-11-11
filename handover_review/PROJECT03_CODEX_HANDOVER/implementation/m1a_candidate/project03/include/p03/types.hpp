#pragma once

#include <compare>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

namespace p03 {
using Lots = std::int64_t;
using Ticks = std::int64_t;
using Money = std::int64_t; // Integer fixture money units; no SDK monetary conversion in M1A.
using LogicalTime = std::uint64_t;
constexpr Lots max_input_lots = 1'000'000;
constexpr Ticks max_input_ticks = 1'000'000'000;
constexpr Money max_money = 1'000'000'000'000'000;

template<class Tag> struct Id {
    std::uint64_t instance{};
    std::uint64_t serial{};
    [[nodiscard]] bool valid() const noexcept { return instance != 0 && serial != 0; }
    auto operator<=>(const Id&) const = default;
};
struct OrderTag; struct CommandTag; struct CancelTag; struct ReservationTag;
using OrderId = Id<OrderTag>;
using CommandId = Id<CommandTag>;
using CancelActionId = Id<CancelTag>;
using ReservationId = Id<ReservationTag>;

template<class Tag> class IdSequence {
public:
    explicit IdSequence(std::uint64_t instance, std::uint64_t next = 1)
        : instance_(instance), next_(next) {
        if (instance == 0 || next == 0) throw std::invalid_argument("invalid identity seed");
    }
    [[nodiscard]] std::optional<Id<Tag>> take() noexcept {
        if (exhausted_) return std::nullopt;
        const Id<Tag> id{instance_, next_};
        if (next_ == std::numeric_limits<std::uint64_t>::max()) exhausted_ = true;
        else ++next_;
        return id;
    }
    [[nodiscard]] std::uint64_t next_serial() const noexcept { return next_; }
    [[nodiscard]] bool exhausted() const noexcept { return exhausted_; }
private:
    std::uint64_t instance_, next_;
    bool exhausted_{};
};

struct TradingDay {
    int ymd{};
    [[nodiscard]] bool valid() const noexcept;
    auto operator<=>(const TradingDay&) const = default;
};
enum class Side { Buy, Sell };
enum class Offset { Open, CloseToday, CloseYesterday };
enum class Bucket : std::size_t { LongToday, LongYesterday, ShortToday, ShortYesterday };
enum class EvidenceOrigin { Unverified, LocalFixture, ExternalUnconfigured };
enum class Channel { Push, Query }; // NOT part of an economic trade key.
enum class Availability { Implemented, NotImplemented, Blocked };
enum class Error {
    None, InvalidInput, ArithmeticOverflow, WrongIdentity, WrongTradingDay,
    NoContract, ClosedSession, PriceRule, QuantityRule, StaleMark, RateLimit,
    PositionLimit, GrossLimit, InsufficientClose, InsufficientFunds, SelfTrade,
    Capacity, IdExhausted, Halted, UntrustedEvidence, EvidenceConflict,
    UnknownOrder, Duplicate, NotImplemented, Blocked, InjectedFailure, OutOfOrderTime
};
[[nodiscard]] std::string_view error_name(Error) noexcept;
[[nodiscard]] std::int64_t checked_add(std::int64_t a, std::int64_t b);
[[nodiscard]] std::int64_t checked_mul_nonnegative(std::int64_t a, std::int64_t b);

struct EvidenceStamp {
    EvidenceOrigin origin{EvidenceOrigin::Unverified};
    std::string source;
    std::uint64_t epoch{};
    std::optional<std::uint64_t> sequence;
    bool comparable{}; // Only the local test profile can assert this in M1A.
    bool operator==(const EvidenceStamp&) const = default;
};
struct ObservationMetadata {
    std::string source, profile, scope, currency;
    LogicalTime request_begin{}, response_end{};
    bool operator==(const ObservationMetadata&) const = default;
};
struct ExternalOrderIdentity {
    std::string exchange;
    std::string order_system_id;
    std::optional<int> front_id;
    std::optional<int> session_id;
    std::string order_ref;
}; // Optional external facts, never fabricated from OrderId.

struct OrderIntent {
    std::string account;
    std::string instrument;
    TradingDay day;
    Side side{Side::Buy};
    Offset offset{Offset::Open};
    Ticks price{};
    Lots quantity{};
    bool operator==(const OrderIntent&) const = default;
};
struct QuantityParts {
    Lots booked{}, suspense{}, possible{}, released{}, unresolved{}; // B,H,P,L,V
    [[nodiscard]] Lots held() const { return checked_add(suspense, checked_add(possible, unresolved)); }
    [[nodiscard]] Lots total() const { return checked_add(booked, checked_add(held(), released)); }
    bool operator==(const QuantityParts&) const = default;
};
} // namespace p03
