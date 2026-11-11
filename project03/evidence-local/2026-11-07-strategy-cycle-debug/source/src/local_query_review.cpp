#include "p03/local_query_review.hpp"
#include <set>
namespace p03 {
LocalQueryReview review_local_query(const SimQuerySnapshot& q,const SimConfig& c){
    const auto invalid=[](Error e){return LocalQueryReview{LocalQueryStatus::Invalid,e};};
    if(c.max_orders==0 || c.max_orders>4096 || c.max_trades==0 || c.max_trades>4096)return invalid(Error::InvalidInput);
    if(!q.query.valid() || q.origin!=EvidenceOrigin::LocalSimulator || q.model_epoch!=c.model_epoch ||
       q.account!=c.account || q.instrument!=c.rules.instrument || q.day!=c.rules.day)return invalid(Error::WrongIdentity);
    if(q.expected_orders>c.max_orders || q.expected_trades>c.max_trades ||
       q.orders.size()>c.max_orders || q.trades.size()>c.max_trades)return invalid(Error::Capacity);
    if(!q.complete || q.orders.size()!=q.expected_orders || q.trades.size()!=q.expected_trades)
        return {LocalQueryStatus::Incomplete,Error::Blocked};
    std::map<OrderId,const SimOrderRecord*> orders;std::map<OrderId,Lots> filled;
    std::set<std::string> trades;auto positions=c.initial_positions;std::array<Lots,4> frozen{};
    for(const auto& o:q.orders){
        const auto& r=o.request;const auto& i=r.intent;
        if(!r.order.valid() || !r.strategy.valid() || (r.client_id && !r.client_id->valid()) ||
           i.account!=c.account || i.instrument!=c.rules.instrument || i.day!=c.rules.day ||
           !orders.emplace(r.order,&o).second || o.filled<0 || o.filled>max_input_lots)
            return invalid(Error::EvidenceConflict);
        if(o.status==SimOrderStatus::Rejected){if(o.filled!=0)return invalid(Error::EvidenceConflict);continue;}
        if(i.quantity<=0 || i.quantity>max_input_lots || o.filled>i.quantity ||
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
    for(std::size_t i=0;i<4;++i)if(positions[i]<0 || frozen[i]<0 || frozen[i]>positions[i])return invalid(Error::EvidenceConflict);
    if(positions!=q.positions || frozen!=q.frozen || q.available_budget<0 || q.available_budget>c.initial_budget)
        return invalid(Error::EvidenceConflict);
    return {LocalQueryStatus::Complete,Error::None};
}
} // namespace p03
