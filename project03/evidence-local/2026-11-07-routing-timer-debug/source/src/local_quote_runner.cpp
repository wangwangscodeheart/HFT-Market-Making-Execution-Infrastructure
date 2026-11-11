#include "p03/local_quote_runner.hpp"
namespace p03 {
LocalQuoteRunner::LocalQuoteRunner(LocalQuoteRunnerConfig c)
    :core_(c.core),peer_(c.peer),strategy_(c.strategy),input_capacity_(c.max_inputs),call_capacity_(c.max_core_calls),
     per_input_bound_(4*c.core.capacity.orders+8){
    market_max_age_=c.strategy.max_age;
    if(c.core.trade_evidence_mode!=TradeEvidenceMode::Simulator || c.strategy.scope!=core_.scope() ||
       c.strategy.strategy!=c.core.strategy || c.peer.account!=core_.scope().account ||
       c.peer.rules.instrument!=core_.scope().contract.instrument || c.peer.rules.exchange!=core_.scope().contract.exchange ||
       c.peer.rules.day!=core_.scope().day || c.peer.initial_positions!=c.core.anchor.booked ||
       c.core.capacity.orders==0 || c.core.capacity.orders>64 || input_capacity_==0 || input_capacity_>1024 ||
       call_capacity_<per_input_bound_ || call_capacity_>65536)throw std::invalid_argument("explicit bounded local runner configuration required");
    trace_.reserve(input_capacity_);
}
StepResult LocalQuoteRunner::call(LocalQuoteFrame& f,const BusinessEvent& event){
    if(f.calls.size()>=per_input_bound_)throw std::logic_error("local runner call bound");
    f.calls.push_back({event,{}}); // Allocate/copy payload before entering core.
    const auto result=core_.step(f.calls.back().input,f.at);f.calls.back().result=result;++call_count_;return result;
}
Error LocalQuoteRunner::reports(LocalQuoteFrame& f,const SimReply& reply){
    // Venue business rejection may still carry terminal evidence that must be delivered.
    for(const auto& event:reply.reports){const auto result=call(f,event);if(result.error!=Error::None && result.error!=Error::Duplicate)return result.error;}
    return reply.reports.empty()?reply.error:Error::None;
}
Error LocalQuoteRunner::cancel(LocalQuoteFrame& f,OrderId id){
    const auto canceled=call(f,RequestCancel{id});
    if(canceled.error!=Error::None || !canceled.cancel)return canceled.error==Error::None?Error::Blocked:canceled.error;
    return reports(f,peer_.cancel(id,*canceled.cancel));
}
Error LocalQuoteRunner::withdraw(LocalQuoteFrame& f){
    std::vector<OrderId> ids;ids.reserve(core_.state().orders.size());
    for(const auto& [id,order]:core_.state().orders)if(order.parts().held()>0 &&
        order.cancel()!=CancelState::Requested && order.cancel()!=CancelState::Unknown)ids.push_back(id);
    for(const auto id:ids){const auto error=cancel(f,id);if(error!=Error::None)return error;}
    return Error::None;
}
Error LocalQuoteRunner::market(LocalQuoteFrame& f,const MarketPulse& pulse){
    if(generation_==std::numeric_limits<std::uint64_t>::max())return Error::IdExhausted;
    ++generation_;
    f.decision=strategy_.evaluate(core_,pulse.book,pulse.session,f.at,generation_);
    if(f.decision->withdraw){const auto error=withdraw(f);return error==Error::None?f.decision->error:error;}
    if(!f.decision->target)return Error::Blocked;
    last_market_at_=pulse.book.received_at;
    // Mark is an explicit recorded core input, derived only from an accepted top.
    const auto mark=pulse.book.bid+(pulse.book.ask-pulse.book.bid)/2;
    const auto marked=call(f,ReferenceMark{mark});if(marked.error!=Error::None)return marked.error;
    for(int pass=0;pass<2;++pass){
        const auto plan=plan_quotes(core_,*f.decision->target,generation_);if(plan.error!=Error::None)return plan.error;
        bool canceled=false;
        for(const auto& intent:plan.intents){
            if(const auto* old=std::get_if<RequestCancel>(&intent)){
                const auto error=cancel(f,old->order);if(error!=Error::None)return error;canceled=true;
            }else{
                const auto& input=std::get<SubmitOrder>(intent);const auto submitted=call(f,input);
                if(submitted.error!=Error::None || !submitted.applied || !submitted.order)
                    return submitted.error==Error::None?Error::Blocked:submitted.error;
                const auto error=reports(f,peer_.receive({*submitted.order,input.intent,core_.config().strategy,input.client_id}));
                if(error!=Error::None)return error;
            }
        }
        if(!canceled)break; // Second pass only after actual synchronous cancel evidence.
    }
    return Error::None;
}
Error LocalQuoteRunner::step(LocalQuoteInput input,LogicalTime at){
    if(stopped_)return Error::Halted;
    if(!trace_.empty() && at<last_time_)return Error::OutOfOrderTime;
    if(trace_.size()>=input_capacity_ || call_count_>call_capacity_-per_input_bound_){stopped_=true;return Error::Capacity;}
    LocalQuoteFrame frame;frame.ordinal=static_cast<std::uint64_t>(trace_.size()+1);frame.at=at;frame.input=std::move(input);
    frame.calls.reserve(per_input_bound_);trace_.push_back(std::move(frame));auto& f=trace_.back();last_time_=at;
    try{
        if(const auto* pulse=std::get_if<MarketPulse>(&f.input))f.error=market(f,*pulse);
        else if(const auto* fill=std::get_if<LocalFillInput>(&f.input))f.error=reports(f,peer_.execute_fill(fill->order,fill->trade_id,fill->quantity,fill->price));
        else if(std::holds_alternative<LocalQuoteTimer>(f.input)){
            if(last_market_at_ && f.at-*last_market_at_>market_max_age_){
                const auto error=withdraw(f);f.error=error==Error::None?Error::StaleMark:error;
            }
        }
        else {f.error=call(f,KillRequested{}).error;if(f.error==Error::None)f.error=withdraw(f);}
        f.canonical=core_.canonical_state();const auto q=peer_.query({core_.config().instance,f.ordinal});f.peer=q.snapshot;
        f.mode=core_.state().mode;f.positions=core_.state().positions.booked();
        for(const auto& [id,order]:core_.state().orders)f.orders.push_back({id,order.intent(),order.parts(),order.execution()});
        if(!core_.invariant() || !peer_.invariant() || q.error!=Error::None || !q.snapshot)throw std::logic_error("local runner invariant");
        f.completed=true;
    }catch(...){f.error=Error::InjectedFailure;stopped_=true;throw;}
    return f.error;
}
} // namespace p03
