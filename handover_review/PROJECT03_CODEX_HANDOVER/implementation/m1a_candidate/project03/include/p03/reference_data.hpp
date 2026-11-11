#pragma once
#include "p03/types.hpp"
#include <vector>

namespace p03 {
struct SessionWindow {
    TradingDay civil_date;
    int begin_second{}; // Beijing civil time, [begin,end).
    int end_second{};
    TradingDay trading_day;
};
struct SessionPoint { TradingDay civil_date; int second{}; TradingDay trading_day; };
class TradingCalendar {
public:
    TradingCalendar(std::string version, std::vector<SessionWindow> windows);
    [[nodiscard]] Error validate(const SessionPoint&, TradingDay expected) const noexcept;
    [[nodiscard]] const std::string& version() const noexcept { return version_; }
private:
    std::string version_;
    std::vector<SessionWindow> windows_;
};
struct Contract {
    std::string instrument;
    std::string product;
    std::string exchange;
    std::string version;
    TradingDay day;
    Ticks low{}, high{};
    Ticks tick_step{1}; // Integer tick coordinates; explicit step permits validation tests.
    Lots min_lots{1}, max_lots{1}, lot_step{1};
    bool synthetic_fixture{true};
    [[nodiscard]] Error validate(const OrderIntent&) const noexcept;
    void check_configuration() const;
};
} // namespace p03
