#include "p03/quote_strategy.hpp"
#include <algorithm>
namespace p03 {
QuoteStrategy::QuoteStrategy(QuoteStrategyConfig c):config_(std::move(c)){
    if(!config_.strategy.valid() || config_.source.empty() || config_.source.size()>128 || config_.market_epoch==0 ||
       config_.max_age==0 || config_.quantity<=0 || config_.quantity>max_input_lots ||
       config_.inventory_limit<=0 || config_.inventory_limit>max_input_lots ||
       (config_.origin!=EvidenceOrigin::LocalFixture && config_.origin!=EvidenceOrigin::LocalSimulator))
        throw std::invalid_argument("explicit local quote strategy configuration required");
}
QuoteDecision QuoteStrategy::evaluate(const CoreLoop& core,const MarketTop& book,SessionPoint session,LogicalTime now,std::uint64_t generation){
    auto stop=[](Error error){return QuoteDecision{error,true,{}};};
    if(config_.scope!=core.scope() || config_.strategy!=core.config().strategy || book.scope!=config_.scope)return stop(Error::WrongIdentity);
    if(book.origin!=config_.origin || book.source!=config_.source || book.epoch!=config_.market_epoch)return stop(Error::UntrustedEvidence);
    if(!book.healthy)return stop(Error::Blocked);
    if(book.sequence==0 || generation==0)return stop(Error::InvalidInput);
    if(book.sequence<=last_sequence_)return stop(Error::Duplicate);
    if(generation<=last_generation_)return stop(Error::Blocked);
    if(book.received_at>now || now-book.received_at>config_.max_age)return stop(Error::StaleMark);
    if(book.bid<=0 || book.ask<=book.bid || book.ask>max_input_ticks || book.bid_quantity<=0 ||
       book.ask_quantity<=0 || book.bid_quantity>max_input_lots || book.ask_quantity>max_input_lots)return stop(Error::InvalidInput);
    const auto& c=core.config();
    const auto session_error=c.calendar.validate(session,c.contract.day);if(session_error!=Error::None)return stop(session_error);
    for(const auto price:{book.bid,book.ask}){
        const auto error=c.contract.validate({core.scope().account,c.contract.instrument,c.contract.day,Side::Buy,Offset::Open,price,config_.quantity});
        if(error!=Error::None)return stop(error);
    }
    const auto& state=core.state();
    if(state.mode!=CoreMode::Ready || state.unknown_exposure || state.risk_deficit)return stop(Error::Halted);
    for(const auto& [id,order]:state.orders){(void)id;if(order.consistency()!=Consistency::InSync)return stop(Error::Blocked);}
    const auto net=core.risk_view().booked_net;
    QuoteTarget target{core.scope(),c.strategy,generation,session,{},{}};
    const auto level=[&](Ticks price,Lots headroom)->std::optional<QuoteLevel>{
        if(headroom<=0)return {};
        auto quantity=std::min(config_.quantity,headroom);
        quantity-=quantity%c.contract.lot_step;
        if(quantity<c.contract.min_lots)return {};
        return QuoteLevel{price,quantity};
    };
    target.buy=level(book.bid,checked_add(config_.inventory_limit,-net));
    target.sell=level(book.ask,checked_add(config_.inventory_limit,net));
    last_sequence_=book.sequence;last_generation_=generation;
    return {Error::None,false,std::move(target)};
}
} // namespace p03
