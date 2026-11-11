#include "p03/risk.hpp"
#include <algorithm>

namespace p03 {
RiskEngine::RiskEngine(RiskLimits l) : limits_(l) {
    if(l.max_order<=0 || l.max_order>max_input_lots || l.max_abs_net<0 || l.max_abs_net>max_input_lots ||
       l.max_gross<0 || l.max_gross>4*max_input_lots || l.local_funds<0 || l.local_funds>max_money ||
       l.safety_buffer<0 || l.safety_buffer>max_money || l.opening_margin_per_lot<0 ||
       l.opening_margin_per_lot>1'000'000 || l.fee_per_lot<0 || l.fee_per_lot>1'000'000 ||
       l.anchor_margin<0 || l.anchor_margin>max_money || l.max_deviation<0 ||
       l.max_deviation>max_input_ticks || l.max_mark_age==0 || l.order_rate==0 ||
       l.order_rate>100'000 || l.rate_window==0) throw std::invalid_argument("risk configuration");
}
RiskView RiskEngine::calculate(const PositionLedger& p,const std::map<OrderId,Order>& orders,
    const ExecutionLedger& ledger,bool unknown) const {
    RiskView v;
    v.booked_net=p.net(); v.min_net=v.max_net=v.booked_net; v.gross_bound=p.gross();
    v.unknown=unknown; v.position_deficit=p.deficit();
    for(const auto& [id,o]:orders) {
        (void)id;
        const auto q=o.parts().held(); // Both H and all unexecuted uncertainty consume the interval.
        if(o.intent().side==Side::Buy) v.max_net=checked_add(v.max_net,q);
        else v.min_net=checked_add(v.min_net,-q);
        if(o.intent().offset==Offset::Open) v.gross_bound=checked_add(v.gross_bound,q);
    }
    for(const auto& [key,r]:ledger.records()) {
        (void)key;
        if(r.disposition!=LedgerDisposition::Exception) continue;
        const auto q=r.trade.quantity;
        v.exception_lots=checked_add(v.exception_lots,q);
        if(r.trade.side==Side::Buy) v.max_net=checked_add(v.max_net,q);
        else v.min_net=checked_add(v.min_net,-q);
        v.gross_bound=checked_add(v.gross_bound,q); // Conservative even for an anomalous close.
    }
    return v;
}
std::vector<FundResource> RiskEngine::resources(const PositionLedger& p,
    const std::map<OrderId,Order>& orders,const ExecutionLedger& ledger,ReservationId anchor) const {
    (void)p;
    std::vector<FundResource> out;
    out.reserve(orders.size()+1);
    out.push_back({anchor,0,limits_.anchor_margin,0});
    for(const auto& [id,o]:orders) {
        const auto held=o.parts().held();
        const auto x=ledger.exception_quantity(id);
        FundResource r{o.reservation(),0,0,0};
        if(o.intent().offset==Offset::Open) {
            r.order_margin=checked_mul_nonnegative(held,limits_.opening_margin_per_lot);
            r.position_margin=checked_mul_nonnegative(checked_add(o.booked(),x),limits_.opening_margin_per_lot);
        } else {
            // Exception closes cannot be used as a credit against local margin.
            r.position_margin=checked_mul_nonnegative(x,limits_.opening_margin_per_lot);
        }
        r.charges=checked_mul_nonnegative(checked_add(checked_add(held,o.booked()),x),limits_.fee_per_lot);
        out.push_back(r);
    }
    return out; // Booked opening margin remains until an external re-anchor (not M1A).
}
Error RiskEngine::check(const RiskView& v,const FundsView& f) const noexcept {
    if(v.unknown) return Error::Halted;
    if(v.position_deficit) return Error::InsufficientClose;
    if(v.min_net < -limits_.max_abs_net || v.max_net>limits_.max_abs_net) return Error::PositionLimit;
    if(v.gross_bound>limits_.max_gross) return Error::GrossLimit;
    if(!f.sufficient()) return Error::InsufficientFunds;
    return Error::None;
}
bool RiskEngine::crosses(const OrderIntent& i,const std::map<OrderId,Order>& orders) const {
    for(const auto& [id,o]:orders) {
        (void)id;
        if(o.intent().side==i.side || o.parts().held()==0) continue;
        if((i.side==Side::Buy && i.price>=o.intent().price) ||
           (i.side==Side::Sell && i.price<=o.intent().price)) return true;
    }
    return false;
}
} // namespace p03
