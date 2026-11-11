#include "p03/types.hpp"
#include <chrono>

namespace p03 {
bool TradingDay::valid() const noexcept {
    if (ymd < 19000101 || ymd > 99991231) return false;
    const auto year = std::chrono::year{ymd / 10000};
    const auto month = std::chrono::month{static_cast<unsigned>((ymd / 100) % 100)};
    const auto day = std::chrono::day{static_cast<unsigned>(ymd % 100)};
    return std::chrono::year_month_day{year,month,day}.ok();
}
std::int64_t checked_add(std::int64_t a, std::int64_t b) {
    if ((b > 0 && a > std::numeric_limits<std::int64_t>::max() - b) ||
        (b < 0 && a < std::numeric_limits<std::int64_t>::min() - b))
        throw std::overflow_error("integer addition");
    return a + b;
}
std::int64_t checked_mul_nonnegative(std::int64_t a, std::int64_t b) {
    if (a < 0 || b < 0) throw std::invalid_argument("negative product");
    if (a != 0 && b > std::numeric_limits<std::int64_t>::max()/a)
        throw std::overflow_error("integer product");
    return a*b;
}
std::string_view error_name(Error e) noexcept {
    switch(e) {
#define P03_ERROR(x) case Error::x: return #x;
        P03_ERROR(None) P03_ERROR(InvalidInput) P03_ERROR(ArithmeticOverflow)
        P03_ERROR(WrongIdentity) P03_ERROR(WrongTradingDay) P03_ERROR(NoContract)
        P03_ERROR(ClosedSession) P03_ERROR(PriceRule) P03_ERROR(QuantityRule)
        P03_ERROR(StaleMark) P03_ERROR(RateLimit) P03_ERROR(PositionLimit)
        P03_ERROR(GrossLimit) P03_ERROR(InsufficientClose) P03_ERROR(InsufficientFunds)
        P03_ERROR(SelfTrade) P03_ERROR(Capacity) P03_ERROR(IdExhausted) P03_ERROR(Halted)
        P03_ERROR(UntrustedEvidence) P03_ERROR(EvidenceConflict) P03_ERROR(UnknownOrder)
        P03_ERROR(Duplicate) P03_ERROR(NotImplemented) P03_ERROR(Blocked)
        P03_ERROR(InjectedFailure) P03_ERROR(OutOfOrderTime)
#undef P03_ERROR
    }
    return "InvalidError";
}
} // namespace p03
