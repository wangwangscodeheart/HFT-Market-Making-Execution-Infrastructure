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
    auto routed=[&](Side side,const QuoteLevel& q,const Order* existing=nullptr){
        auto offset=Offset::Open;Lots quantity=q.quantity;
        const auto today=side==Side::Buy?2U:0U;
        auto yesterday_available=positions[today+1]-frozen[today+1];
        auto today_available=positions[today]-frozen[today];
        // Compare as if this quote's own remaining close reservation were released.
        if(existing && existing->intent().offset!=Offset::Open){
            const auto remaining=existing->intent().quantity-existing->booked();
            if(existing->intent().offset==Offset::CloseYesterday)yesterday_available=checked_add(yesterday_available,remaining);
            else today_available=checked_add(today_available,remaining);
        }
        // Explicit project preference: yesterday before today; not an exchange rule.
        if(yesterday_available>0){offset=Offset::CloseYesterday;quantity=std::min(quantity,yesterday_available);}
        else if(today_available>0){offset=Offset::CloseToday;quantity=std::min(quantity,today_available);}
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
        const auto desired_intent=q?std::optional<OrderIntent>{routed(order.intent().side,*q,&order)}:std::nullopt;
        if(!desired_intent || order.intent().price!=desired_intent->price || order.intent().offset!=desired_intent->offset ||
           order.intent().quantity-order.booked()!=desired_intent->quantity){
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
