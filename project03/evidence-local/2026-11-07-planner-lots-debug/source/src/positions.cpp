#include "p03/positions.hpp"

namespace p03 {
namespace {
std::size_t index(Bucket b) {
    const auto i=static_cast<std::size_t>(b);
    if(i>=4) throw std::invalid_argument("invalid bucket");
    return i;
}
}
PositionLedger::PositionLedger(PositionAnchor a) : anchor_(std::move(a)), booked_(anchor_.booked) {
    if(anchor_.account.empty() || anchor_.account.size()>128 || anchor_.instrument.empty() ||
       anchor_.instrument.size()>128 || !anchor_.day.valid() || anchor_.epoch==0 ||
       anchor_.trade_coverage.empty() || anchor_.trade_coverage.size()>128)
        throw std::invalid_argument("explicit position anchor required");
    for(auto q:booked_) if(q<0 || q>max_input_lots) throw std::invalid_argument("position anchor quantity");
}
Lots PositionLedger::net() const { return checked_add(checked_add(booked_[0],booked_[1]),-checked_add(booked_[2],booked_[3])); }
Lots PositionLedger::gross() const { return checked_add(checked_add(booked_[0],booked_[1]),checked_add(booked_[2],booked_[3])); }
Lots PositionLedger::available(Bucket b) const { return checked_add(booked_[index(b)],-frozen_[index(b)]); }
bool PositionLedger::deficit() const noexcept {
    for(std::size_t i=0;i<4;++i) if(booked_[i]<0 || frozen_[i]<0 || frozen_[i]>booked_[i]) return true;
    return false;
}
void PositionLedger::replace_freezes(const std::array<Lots,4>& freezes) {
    for(auto q:freezes) if(q<0) throw std::invalid_argument("negative freeze");
    frozen_=freezes;
}
Bucket PositionLedger::affected_bucket(Side side, Offset off) noexcept {
    if(off==Offset::Open) return side==Side::Buy ? Bucket::LongToday : Bucket::ShortToday;
    if(side==Side::Buy) return off==Offset::CloseToday ? Bucket::ShortToday : Bucket::ShortYesterday;
    return off==Offset::CloseToday ? Bucket::LongToday : Bucket::LongYesterday;
}
void PositionLedger::book(Side side, Offset off, Lots q) {
    if(q<=0 || q>max_input_lots || (side!=Side::Buy && side!=Side::Sell) ||
       (off!=Offset::Open && off!=Offset::CloseToday && off!=Offset::CloseYesterday))
        throw std::invalid_argument("invalid execution quantity/side/offset");
    const auto i=index(affected_bucket(side,off));
    booked_[i]=checked_add(booked_[i],off==Offset::Open ? q : -q);
}
} // namespace p03
