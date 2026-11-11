#include "p03/order_planner.hpp"
#include <algorithm>
namespace p03 {
QuotePlan plan_quotes(const CoreLoop& core,const QuoteTarget& target,std::uint64_t current_generation){
    QuotePlan plan;plan.generation=target.generation;
    auto fail=[&](Error error){plan.error=error;plan.intents.clear();return plan;};
    if(target.scope!=core.scope() || target.strategy!=core.config().strategy)return fail(Error::WrongIdentity);
    if(target.generation==0 || target.generation!=current_generation)return fail(Error::Blocked);
    const auto& state=core.state();
    if(state.mode!=CoreMode::Ready || state.unknown_exposure || state.risk_deficit)return fail(Error::Halted);
    const auto valid=[](const std::optional<QuoteLevel>& q){return !q || (q->price>0 && q->price<=max_input_ticks && q->quantity>0 && q->quantity<=max_input_lots);};
    if(!valid(target.buy) || !valid(target.sell))return fail(Error::InvalidInput);
    if(target.buy && target.sell && target.buy->price>=target.sell->price)return fail(Error::SelfTrade);
    std::array<const Order*,2> active{};
    for(const auto& [id,order]:state.orders){
        (void)id;
        if(order.consistency()!=Consistency::InSync)return fail(Error::Blocked);
        const auto p=order.parts();if(p.held()==0)continue;
        const auto index=order.intent().side==Side::Buy?0U:1U;
        if(active[index])return fail(Error::Blocked); // No silent choice among multiple live quotes.
        active[index]=&order;
    }
    const auto& positions=state.positions.booked();const auto& frozen=state.positions.frozen();
    auto routed=[&](Side side,const QuoteLevel& q){
        auto offset=Offset::Open;Lots quantity=q.quantity;
        const auto today=side==Side::Buy?2U:0U;
        // Explicit project preference: yesterday before today; not an exchange rule.
        if(positions[today+1]-frozen[today+1]>0){offset=Offset::CloseYesterday;quantity=std::min(quantity,positions[today+1]-frozen[today+1]);}
        else if(positions[today]-frozen[today]>0){offset=Offset::CloseToday;quantity=std::min(quantity,positions[today]-frozen[today]);}
        return OrderIntent{core.scope().account,core.scope().contract.instrument,core.scope().day,side,offset,q.price,quantity};
    };
    const std::array<std::optional<QuoteLevel>,2> desired{target.buy,target.sell};
    // Cancel pass precedes creation. Pending cancel continues to occupy its slot.
    bool wait=false;
    for(std::size_t i=0;i<2;++i){
        if(!active[i])continue;
        const auto& order=*active[i];
        if(order.cancel()==CancelState::Requested || order.cancel()==CancelState::Unknown){wait=true;continue;}
        const auto& q=desired[i];
        // Retain unchanged price/quantity; do not reroute an already frozen close.
        if(!q || order.intent().price!=q->price || order.intent().quantity-order.booked()!=q->quantity){
            plan.intents.emplace_back(RequestCancel{order.id()});wait=true;
        }
    }
    if(wait)return plan;
    for(std::size_t i=0;i<2;++i)if(!active[i] && desired[i]){
        auto intent=routed(i==0?Side::Buy:Side::Sell,*desired[i]);
        plan.intents.emplace_back(SubmitOrder{std::move(intent),target.session,{},target.strategy});
    }
    return plan;
}
} // namespace p03
