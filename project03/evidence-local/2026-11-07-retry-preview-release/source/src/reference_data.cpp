#include "p03/reference_data.hpp"
#include <algorithm>

namespace p03 {
TradingCalendar::TradingCalendar(std::string v, std::vector<SessionWindow> w)
    : version_(std::move(v)), windows_(std::move(w)) {
    if (version_.empty() || version_.size()>128 || windows_.empty() || windows_.size()>4096)
        throw std::invalid_argument("calendar configuration");
    for (const auto& x: windows_) {
        if (!x.civil_date.valid() || !x.trading_day.valid() || x.begin_second<0 ||
            x.end_second>86400 || x.begin_second>=x.end_second)
            throw std::invalid_argument("session window");
    }
    std::sort(windows_.begin(),windows_.end(),[](const auto& a,const auto& b){
        return std::pair{a.civil_date,a.begin_second}<std::pair{b.civil_date,b.begin_second};
    });
    for (std::size_t i=1;i<windows_.size();++i)
        if(windows_[i-1].civil_date==windows_[i].civil_date &&
           windows_[i-1].end_second>windows_[i].begin_second)
            throw std::invalid_argument("overlapping session windows");
}
Error TradingCalendar::validate(const SessionPoint& p, TradingDay expected) const noexcept {
    if(!p.civil_date.valid() || !p.trading_day.valid() || p.second<0 || p.second>=86400)
        return Error::InvalidInput;
    if(p.trading_day!=expected) return Error::WrongTradingDay;
    for(const auto& w:windows_)
        if(w.civil_date==p.civil_date && p.second>=w.begin_second && p.second<w.end_second)
            return w.trading_day==p.trading_day ? Error::None : Error::WrongTradingDay;
    return Error::ClosedSession;
}
void Contract::check_configuration() const {
    if(instrument.empty() || instrument.size()>128 || instrument==product ||
       product!="RB" || exchange!="SHFE" || version.empty() || version.size()>128 || !day.valid() ||
       low<=0 || high<low || high>max_input_ticks || tick_step<=0 ||
       low%tick_step!=0 || high%tick_step!=0 || min_lots<=0 || max_lots<min_lots ||
       max_lots>max_input_lots || lot_step<=0 || min_lots%lot_step!=0)
        throw std::invalid_argument("explicit SHFE RB contract configuration required");
}
Error Contract::validate(const OrderIntent& x) const noexcept {
    if(x.instrument!=instrument) return Error::NoContract;
    if(x.day!=day) return Error::WrongTradingDay;
    if(x.price<low || x.price>high || x.price%tick_step!=0) return Error::PriceRule;
    if(x.quantity<min_lots || x.quantity>max_lots || x.quantity%lot_step!=0) return Error::QuantityRule;
    return Error::None;
}
} // namespace p03
