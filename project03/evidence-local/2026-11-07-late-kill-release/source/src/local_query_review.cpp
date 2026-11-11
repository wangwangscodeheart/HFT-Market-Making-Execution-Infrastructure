#include "p03/local_query_review.hpp"
#include <set>
namespace p03 {
LocalQueryReview review_local_query_for_core(const SimQueryResult& result,QueryId expected,const SimConfig& config,const CoreLoop& core){
    if(config.account!=core.scope().account || config.rules.instrument!=core.scope().contract.instrument ||
       config.rules.exchange!=core.scope().contract.exchange || config.rules.day!=core.scope().day || config.initial_positions!=core.config().anchor.booked)
        return {LocalQueryStatus::Invalid,Error::WrongIdentity};
    const auto review=review_local_query(result,expected,config);if(review.status!=LocalQueryStatus::Complete)return review;
    const auto& q=*result.snapshot;std::set<OrderId> seen;
    for(const auto& order:q.orders){
        const auto found=core.state().orders.find(order.request.order);
        if(found==core.state().orders.end())return {LocalQueryStatus::Invalid,Error::UnknownOrder};
        const auto& local=found->second;
        if(order.request.intent!=local.intent() || order.request.strategy!=local.strategy() || order.request.client_id!=local.client_id())
            return {LocalQueryStatus::Invalid,Error::EvidenceConflict};
        if(order.filled<local.booked())return {LocalQueryStatus::Incomplete,Error::Blocked};
        seen.insert(order.request.order);
    }
    for(const auto& [id,order]:core.state().orders){(void)order;if(!seen.contains(id))return {LocalQueryStatus::Incomplete,Error::Blocked};}
    std::map<TradeKey,Trade> trades;for(const auto& report:q.trades)trades.emplace(report.trade.key,report.trade);
    for(const auto& [key,record]:core.state().executions.records()){
        const auto found=trades.find(key);if(found==trades.end())return {LocalQueryStatus::Incomplete,Error::Blocked};
        if(found->second!=record.trade)return {LocalQueryStatus::Invalid,Error::EvidenceConflict};
    }
    return review;
}
LocalQueryReview review_local_query(const SimQueryResult& result,QueryId expected,const SimConfig& config){
    if(!expected.valid())return {LocalQueryStatus::Invalid,Error::InvalidInput};
    if(result.error!=Error::None)return {LocalQueryStatus::Invalid,result.error};
    if(!result.snapshot)return {LocalQueryStatus::Incomplete,Error::Blocked};
    if(result.snapshot->query!=expected)return {LocalQueryStatus::Invalid,Error::WrongIdentity};
    return review_local_query(*result.snapshot,config);
}
LocalQueryReview review_local_query(const SimQuerySnapshot& q,const SimConfig& c){
    const auto invalid=[](Error e){return LocalQueryReview{LocalQueryStatus::Invalid,e};};
    if(c.max_orders==0 || c.max_orders>4096 || c.max_trades==0 || c.max_trades>4096 ||
       c.initial_budget<0 || c.initial_budget>max_money || c.opening_margin_per_lot<0 || c.opening_margin_per_lot>1'000'000 ||
       c.fee_per_lot<0 || c.fee_per_lot>1'000'000 || c.rules.tick_step<=0 || c.rules.lot_step<=0 ||
       c.rules.min_lots<=0 || c.rules.max_lots<c.rules.min_lots || c.rules.max_lots>max_input_lots)return invalid(Error::InvalidInput);
    if(c.rules.low<=0 || c.rules.high<c.rules.low || c.rules.high>max_input_ticks ||
       c.rules.low%c.rules.tick_step!=0 || c.rules.high%c.rules.tick_step!=0 || c.rules.min_lots%c.rules.lot_step!=0)
        return invalid(Error::InvalidInput);
    Money consumed=0;
    for(const auto quantity:c.initial_positions){
        if(quantity<0 || quantity>max_input_lots)return invalid(Error::InvalidInput);
        consumed=checked_add(consumed,checked_mul_nonnegative(quantity,c.opening_margin_per_lot));
    }
    if(!q.query.valid() || q.origin!=EvidenceOrigin::LocalSimulator || q.model_epoch!=c.model_epoch ||
       q.account!=c.account || q.instrument!=c.rules.instrument || q.day!=c.rules.day)return invalid(Error::WrongIdentity);
    if(q.expected_orders>c.max_orders || q.expected_trades>c.max_trades ||
       q.orders.size()>c.max_orders || q.trades.size()>c.max_trades)return invalid(Error::Capacity);
    if(!q.complete || q.orders.size()!=q.expected_orders || q.trades.size()!=q.expected_trades)
        return {LocalQueryStatus::Incomplete,Error::Blocked};
    if(q.revision==0 && (!q.orders.empty() || !q.trades.empty()))return invalid(Error::EvidenceConflict);
    std::map<OrderId,const SimOrderRecord*> orders;std::map<OrderId,Lots> filled;
    std::set<std::string> trades;auto positions=c.initial_positions;std::array<Lots,4> frozen{};
    for(const auto& o:q.orders){
        const auto& r=o.request;const auto& i=r.intent;
        if(!r.order.valid() || !r.strategy.valid() || (r.client_id && !r.client_id->valid()) ||
           (i.side!=Side::Buy && i.side!=Side::Sell) ||
           (i.offset!=Offset::Open && i.offset!=Offset::CloseToday && i.offset!=Offset::CloseYesterday) ||
           i.account!=c.account || i.instrument!=c.rules.instrument || i.day!=c.rules.day ||
           !orders.emplace(r.order,&o).second || o.filled<0 || o.filled>max_input_lots)
            return invalid(Error::EvidenceConflict);
        if(o.status==SimOrderStatus::Rejected){if(o.filled!=0)return invalid(Error::EvidenceConflict);continue;}
        if(i.quantity<=0 || i.quantity>max_input_lots || o.filled>i.quantity ||
           i.quantity<c.rules.min_lots || i.quantity>c.rules.max_lots || i.quantity%c.rules.lot_step!=0 ||
           i.price<c.rules.low || i.price>c.rules.high || i.price%c.rules.tick_step!=0 ||
           (i.side!=Side::Buy && i.side!=Side::Sell) ||
           (i.offset!=Offset::Open && i.offset!=Offset::CloseToday && i.offset!=Offset::CloseYesterday))
            return invalid(Error::InvalidInput);
        const bool active=o.status==SimOrderStatus::Working || o.status==SimOrderStatus::PartiallyFilled;
        if((o.status==SimOrderStatus::Working && o.filled!=0) ||
           (o.status==SimOrderStatus::PartiallyFilled && (o.filled==0 || o.filled==i.quantity)) ||
           (o.status==SimOrderStatus::Filled && o.filled!=i.quantity) ||
           (!active && o.status!=SimOrderStatus::Filled && o.status!=SimOrderStatus::Canceled))return invalid(Error::EvidenceConflict);
        if(active && i.offset!=Offset::Open){
            const auto b=(i.side==Side::Sell?0U:2U)+(i.offset==Offset::CloseYesterday?1U:0U);
            frozen[b]=checked_add(frozen[b],i.quantity-o.filled);
        }
    }
    for(const auto& report:q.trades){
        const auto& t=report.trade;const auto found=orders.find(t.order);
        if(found==orders.end() || t.key.trade_id.empty() || t.key.trade_id.size()>128 || !trades.insert(t.key.trade_id).second ||
           t.key.account!=c.account || t.key.day!=c.rules.day || t.key.exchange!=c.rules.exchange ||
           t.key.scope!="p03.localsim.day.v1" || t.instrument!=c.rules.instrument || report.channel!=Channel::Query ||
           report.stamp.origin!=EvidenceOrigin::LocalSimulator || report.stamp.epoch!=c.model_epoch ||
           report.stamp.source!="localsim.query.trades" || !report.stamp.comparable || report.stamp.sequence!=q.revision ||
           t.quantity<=0 || t.quantity>max_input_lots || t.price<c.rules.low || t.price>c.rules.high ||
           c.rules.tick_step<=0 || t.price%c.rules.tick_step!=0)return invalid(Error::EvidenceConflict);
        const auto& i=found->second->request.intent;
        if(found->second->status==SimOrderStatus::Rejected || t.side!=i.side || t.offset!=i.offset ||
           (i.side==Side::Buy && t.price>i.price) || (i.side==Side::Sell && t.price<i.price))return invalid(Error::EvidenceConflict);
        filled[t.order]=checked_add(filled[t.order],t.quantity);
        const auto b=t.offset==Offset::Open?(t.side==Side::Buy?0U:2U):
            (t.side==Side::Sell?0U:2U)+(t.offset==Offset::CloseYesterday?1U:0U);
        positions[b]=checked_add(positions[b],t.offset==Offset::Open?t.quantity:-t.quantity);
    }
    for(const auto& [id,order]:orders)if(filled[id]!=order->filled)return {LocalQueryStatus::Incomplete,Error::Blocked};
    // Reconstruct the explicit conservative simulator budget independently.
    // Close fills retain historical margin; this is not exchange equity.
    for(const auto& [id,order]:orders){
        (void)id;const auto& o=*order;
        if(o.reserved_margin<0 || o.reserved_fees<0 || o.position_margin<0 || o.paid_fees<0)return invalid(Error::EvidenceConflict);
        const bool active=o.status==SimOrderStatus::Working || o.status==SimOrderStatus::PartiallyFilled;
        const auto remaining=active?o.request.intent.quantity-o.filled:0;
        const bool opens=o.request.intent.offset==Offset::Open;
        const auto margin=opens?checked_mul_nonnegative(remaining,c.opening_margin_per_lot):0;
        const auto position_margin=opens?checked_mul_nonnegative(o.filled,c.opening_margin_per_lot):0;
        const auto fees=checked_mul_nonnegative(remaining,c.fee_per_lot),paid=checked_mul_nonnegative(o.filled,c.fee_per_lot);
        if(o.reserved_margin!=margin || o.position_margin!=position_margin || o.reserved_fees!=fees || o.paid_fees!=paid)
            return invalid(Error::EvidenceConflict);
        consumed=checked_add(consumed,checked_add(checked_add(margin,position_margin),checked_add(fees,paid)));
    }
    for(std::size_t i=0;i<4;++i)if(positions[i]<0 || frozen[i]<0 || frozen[i]>positions[i])return invalid(Error::EvidenceConflict);
    if(positions!=q.positions || frozen!=q.frozen || consumed>c.initial_budget || q.available_budget!=c.initial_budget-consumed)
        return invalid(Error::EvidenceConflict);
    return {LocalQueryStatus::Complete,Error::None};
}
} // namespace p03
