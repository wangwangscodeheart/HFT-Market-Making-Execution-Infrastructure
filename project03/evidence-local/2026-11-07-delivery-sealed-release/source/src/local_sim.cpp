#include "p03/local_sim.hpp"
#include <algorithm>

namespace p03 {
namespace {
bool bounded(const std::string& value){return !value.empty() && value.size()<=128;}
}
bool LocalSimVenue::active(SimOrderStatus status) noexcept {
    return status==SimOrderStatus::Working || status==SimOrderStatus::PartiallyFilled;
}
std::size_t LocalSimVenue::bucket(Side side,Offset offset){
    if(offset==Offset::Open)return side==Side::Buy?0U:2U;
    return (side==Side::Sell?0U:2U)+(offset==Offset::CloseYesterday?1U:0U);
}
LocalSimVenue::LocalSimVenue(SimConfig config):config_(std::move(config)) {
    const auto& r=config_.rules;
    if(!bounded(config_.account) || !bounded(r.instrument) || r.instrument=="RB" || r.exchange!="SHFE" || !r.day.valid() ||
       r.low<=0 || r.high<r.low || r.high>max_input_ticks || r.tick_step<=0 || r.low%r.tick_step!=0 || r.high%r.tick_step!=0 ||
       r.min_lots<=0 || r.max_lots<r.min_lots || r.max_lots>max_input_lots || r.lot_step<=0 || r.min_lots%r.lot_step!=0 ||
       config_.initial_budget<0 || config_.initial_budget>max_money || config_.opening_margin_per_lot<0 ||
       config_.opening_margin_per_lot>1'000'000 || config_.fee_per_lot<0 || config_.fee_per_lot>1'000'000 || config_.model_epoch==0)
        throw std::invalid_argument("explicit LocalSim configuration required");
    for(const auto limit:{config_.max_orders,config_.max_trades,config_.max_cancels})
        if(limit==0 || limit>4096)throw std::invalid_argument("LocalSim capacity must be in [1,4096]");
    for(const auto q:config_.initial_positions){
        if(q<0 || q>max_input_lots)throw std::invalid_argument("LocalSim initial positions");
        initial_margin_=checked_add(initial_margin_,checked_mul_nonnegative(q,config_.opening_margin_per_lot));
    }
    if(initial_margin_>config_.initial_budget)throw std::invalid_argument("LocalSim initial budget");
    state_.positions=config_.initial_positions;state_.available=config_.initial_budget-initial_margin_;
}
EvidenceStamp LocalSimVenue::stamp(std::uint64_t revision,std::string source) const {
    return {EvidenceOrigin::LocalSimulator,std::move(source),config_.model_epoch,revision,true};
}
Error LocalSimVenue::request_error(const SimOrderRequest& request) const {
    const auto& i=request.intent;const auto& r=config_.rules;
    if(!request.order.valid() || !request.strategy.valid() || (request.client_id && !request.client_id->valid()))return Error::InvalidInput;
    if(i.account!=config_.account || i.instrument!=r.instrument)return Error::WrongIdentity;
    if(i.day!=r.day)return Error::WrongTradingDay;
    if((i.side!=Side::Buy && i.side!=Side::Sell) ||
       (i.offset!=Offset::Open && i.offset!=Offset::CloseToday && i.offset!=Offset::CloseYesterday))return Error::InvalidInput;
    if(i.price<r.low || i.price>r.high || i.price%r.tick_step!=0)return Error::PriceRule;
    if(i.quantity<r.min_lots || i.quantity>r.max_lots || i.quantity%r.lot_step!=0)return Error::QuantityRule;
    return Error::None;
}
void LocalSimVenue::commit(State& candidate){
    if(!invariant(candidate))throw std::logic_error("LocalSim independent accounting invariant failed");
    using std::swap;swap(state_,candidate);
}
SimReply LocalSimVenue::receive(const SimOrderRequest& request){
    const auto found=state_.orders.find(request.order);
    if(found!=state_.orders.end())return {found->second.request==request?Error::Duplicate:Error::EvidenceConflict,state_.revision,{}};
    auto error=request_error(request);
    if(error==Error::InvalidInput || error==Error::WrongIdentity || error==Error::WrongTradingDay)return {error,state_.revision,{}};
    if(state_.orders.size()>=config_.max_orders)return {Error::Capacity,state_.revision,{}};
    if(state_.revision==std::numeric_limits<std::uint64_t>::max())return {Error::IdExhausted,state_.revision,{}};
    Money margin=0,fees=0;
    const auto& i=request.intent;
    if(error==Error::None){
        if(i.offset!=Offset::Open){
            const auto b=bucket(i.side,i.offset);
            if(i.quantity>state_.positions[b]-state_.frozen[b])error=Error::InsufficientClose;
        }else margin=checked_mul_nonnegative(i.quantity,config_.opening_margin_per_lot);
        fees=checked_mul_nonnegative(i.quantity,config_.fee_per_lot);
        if(error==Error::None && checked_add(margin,fees)>state_.available)error=Error::InsufficientFunds;
    }
    auto candidate=state_;++candidate.revision;
    SimOrderRecord record{request,"SIM-"+std::to_string(config_.model_epoch)+"-"+std::to_string(candidate.revision)};
    const EvidenceHeader header{request.order,config_.account,config_.rules.day,stamp(candidate.revision,"localsim.orders")};
    SimReply reply{error,candidate.revision,{}};
    if(error==Error::None){
        record.reserved_margin=margin;record.reserved_fees=fees;
        candidate.available-=checked_add(margin,fees);
        if(i.offset!=Offset::Open)candidate.frozen[bucket(i.side,i.offset)]+=i.quantity;
        reply.reports.emplace_back(WorkingReport{header});
    }else{
        record.status=SimOrderStatus::Rejected;
        reply.reports.emplace_back(TerminalReport{header,{TerminalKind::Rejected,0}});
    }
    candidate.orders.emplace(request.order,std::move(record));commit(candidate);return reply;
}
SimReply LocalSimVenue::execute_fill(OrderId id,std::string trade_id,Lots quantity,Ticks price){
    const auto found=state_.orders.find(id);
    if(found==state_.orders.end())return {Error::UnknownOrder,state_.revision,{}};
    if(!bounded(trade_id) || quantity<=0 || quantity>max_input_lots || price<=0 || price>max_input_ticks)return {Error::InvalidInput,state_.revision,{}};
    const auto& i=found->second.request.intent;
    Trade trade{{config_.account,config_.rules.day,config_.rules.exchange,"p03.localsim.day.v1",trade_id},id,i.instrument,i.side,i.offset,quantity,price};
    const auto previous=state_.trades.find(trade_id);
    if(previous!=state_.trades.end()){
        if(previous->second.trade!=trade)return {Error::EvidenceConflict,state_.revision,{}};
        return {Error::Duplicate,state_.revision,{previous->second}};
    }
    if(!active(found->second.status) || quantity>i.quantity-found->second.filled)return {Error::QuantityRule,state_.revision,{}};
    if(price<config_.rules.low || price>config_.rules.high || price%config_.rules.tick_step!=0 ||
       (i.side==Side::Buy && price>i.price) || (i.side==Side::Sell && price<i.price))return {Error::PriceRule,state_.revision,{}};
    if(state_.trades.size()>=config_.max_trades)return {Error::Capacity,state_.revision,{}};
    if(state_.revision==std::numeric_limits<std::uint64_t>::max())return {Error::IdExhausted,state_.revision,{}};
    auto candidate=state_;++candidate.revision;auto& order=candidate.orders.at(id);
    const auto fees=checked_mul_nonnegative(quantity,config_.fee_per_lot);
    order.reserved_fees-=fees;order.paid_fees=checked_add(order.paid_fees,fees);
    const auto b=bucket(i.side,i.offset);
    if(i.offset==Offset::Open){
        const auto margin=checked_mul_nonnegative(quantity,config_.opening_margin_per_lot);
        order.reserved_margin-=margin;order.position_margin=checked_add(order.position_margin,margin);
        candidate.positions[b]=checked_add(candidate.positions[b],quantity);
    }else{candidate.positions[b]-=quantity;candidate.frozen[b]-=quantity;}
    order.filled+=quantity;order.status=order.filled==i.quantity?SimOrderStatus::Filled:SimOrderStatus::PartiallyFilled;
    TradeReport report{std::move(trade),stamp(candidate.revision,"localsim.trades"),Channel::Push};
    candidate.trades.emplace(std::move(trade_id),report);
    SimReply reply{Error::None,candidate.revision,{report}};commit(candidate);return reply;
}
SimReply LocalSimVenue::cancel(OrderId id,CancelActionId action){
    if(!action.valid() || action.instance!=id.instance)return {Error::WrongIdentity,state_.revision,{}};
    const auto found=state_.orders.find(id);if(found==state_.orders.end())return {Error::UnknownOrder,state_.revision,{}};
    const auto previous=state_.cancels.find(action);
    if(previous!=state_.cancels.end())return previous->second.order==id?
        SimReply{Error::Duplicate,state_.revision,previous->second.reports}:SimReply{Error::EvidenceConflict,state_.revision,{}};
    if(state_.cancels.size()>=config_.max_cancels)return {Error::Capacity,state_.revision,{}};
    if(state_.revision==std::numeric_limits<std::uint64_t>::max())return {Error::IdExhausted,state_.revision,{}};
    auto candidate=state_;++candidate.revision;auto& order=candidate.orders.at(id);
    const EvidenceHeader header{id,config_.account,config_.rules.day,stamp(candidate.revision,"localsim.orders")};
    SimReply reply{Error::None,candidate.revision,{}};
    if(active(order.status)){
        const auto remaining=order.request.intent.quantity-order.filled;
        candidate.available=checked_add(candidate.available,checked_add(order.reserved_margin,order.reserved_fees));
        order.reserved_margin=0;order.reserved_fees=0;
        if(order.request.intent.offset!=Offset::Open)candidate.frozen[bucket(order.request.intent.side,order.request.intent.offset)]-=remaining;
        order.status=SimOrderStatus::Canceled;
        reply.reports={CancelReport{header,action,CancelState::Succeeded},TerminalReport{header,{TerminalKind::Canceled,order.filled}}};
    }else reply.reports={CancelReport{header,action,CancelState::Rejected}};
    candidate.cancels.emplace(action,CancelRecord{id,reply.reports});commit(candidate);return reply;
}
SimQueryResult LocalSimVenue::query(QueryId query_id) const {
    if(!query_id.valid())return {Error::InvalidInput,{}};
    SimQuerySnapshot snapshot{query_id,EvidenceOrigin::LocalSimulator,config_.model_epoch,state_.revision,
        config_.account,config_.rules.instrument,config_.rules.day,state_.positions,state_.frozen,state_.available,{},{}};
    for(const auto& [id,order]:state_.orders){(void)id;snapshot.orders.push_back(order);}
    for(const auto& [id,push]:state_.trades){
        (void)id;auto report=push;report.channel=Channel::Query;report.stamp=stamp(state_.revision,"localsim.query.trades");
        snapshot.trades.push_back(std::move(report));
    }
    return {Error::None,std::move(snapshot)};
}
bool LocalSimVenue::invariant(const State& s) const {
    if(s.orders.size()>config_.max_orders || s.trades.size()>config_.max_trades || s.cancels.size()>config_.max_cancels)return false;
    auto positions=config_.initial_positions;std::array<Lots,4> frozen{};
    Money consumed=initial_margin_;
    for(const auto& [id,report]:s.trades){
        const auto& t=report.trade;
        if(id!=t.key.trade_id || !s.orders.contains(t.order) || report.stamp.origin!=EvidenceOrigin::LocalSimulator)return false;
        const auto b=bucket(t.side,t.offset);
        positions[b]=checked_add(positions[b],t.offset==Offset::Open?t.quantity:-t.quantity);
    }
    for(const auto& [id,order]:s.orders){
        if(id!=order.request.order || order.filled<0 || order.reserved_margin<0 || order.reserved_fees<0 || order.position_margin<0 || order.paid_fees<0)return false;
        if(order.status==SimOrderStatus::Rejected){
            if(order.filled!=0 || order.reserved_margin!=0 || order.reserved_fees!=0 || order.position_margin!=0 || order.paid_fees!=0)return false;
            continue;
        }
        if(order.filled>order.request.intent.quantity)return false;
        Lots filled=0;for(const auto& [key,report]:s.trades){(void)key;if(report.trade.order==id)filled=checked_add(filled,report.trade.quantity);}
        if(filled!=order.filled)return false;
        const auto remaining=active(order.status)?order.request.intent.quantity-order.filled:0;
        if(order.request.intent.offset!=Offset::Open)frozen[bucket(order.request.intent.side,order.request.intent.offset)]+=remaining;
        if(order.reserved_fees!=checked_mul_nonnegative(remaining,config_.fee_per_lot) ||
           order.paid_fees!=checked_mul_nonnegative(order.filled,config_.fee_per_lot))return false;
        const auto expected_order_margin=order.request.intent.offset==Offset::Open?checked_mul_nonnegative(remaining,config_.opening_margin_per_lot):0;
        const auto expected_position_margin=order.request.intent.offset==Offset::Open?checked_mul_nonnegative(order.filled,config_.opening_margin_per_lot):0;
        if(order.reserved_margin!=expected_order_margin || order.position_margin!=expected_position_margin)return false;
        consumed=checked_add(consumed,checked_add(checked_add(order.reserved_margin,order.reserved_fees),checked_add(order.position_margin,order.paid_fees)));
    }
    for(std::size_t i=0;i<4;++i)if(positions[i]<0 || frozen[i]<0 || frozen[i]>positions[i])return false;
    return positions==s.positions && frozen==s.frozen && s.available>=0 && s.available==config_.initial_budget-consumed;
}
bool LocalSimVenue::invariant() const {return invariant(state_);}
} // namespace p03
