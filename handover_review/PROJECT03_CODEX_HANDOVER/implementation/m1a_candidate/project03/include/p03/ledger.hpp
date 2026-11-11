#pragma once
#include "p03/types.hpp"
#include <map>

namespace p03 {
struct TradeKey {
    std::string account;
    TradingDay day;
    std::string exchange;
    std::string scope;
    std::string trade_id;
    auto operator<=>(const TradeKey&) const = default;
};
struct Trade {
    TradeKey key;
    OrderId order;
    std::string instrument;
    Side side{Side::Buy};
    Offset offset{Offset::Open};
    Lots quantity{};
    Ticks price{};
    bool operator==(const Trade&) const = default;
};
enum class LedgerDisposition { Normal, Exception };
struct ExecutionRecord { Trade trade; LedgerDisposition disposition; };
enum class DedupResult { New, Identical, Conflicting };
class ExecutionLedger {
public:
    [[nodiscard]] DedupResult classify(const Trade&) const;
    void insert(Trade, LedgerDisposition);
    [[nodiscard]] const std::map<TradeKey,ExecutionRecord>& records() const noexcept { return records_; }
    [[nodiscard]] Lots normal_quantity(OrderId) const;
    [[nodiscard]] Lots exception_quantity(OrderId) const;
private:
    std::map<TradeKey,ExecutionRecord> records_;
};
} // namespace p03
