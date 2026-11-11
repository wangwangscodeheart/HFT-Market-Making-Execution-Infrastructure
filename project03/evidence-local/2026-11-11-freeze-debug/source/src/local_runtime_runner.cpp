#include "p03/local_runtime_runner.hpp"
namespace p03 {
LocalRuntimeRunner::LocalRuntimeRunner(LocalRuntimeConfig c)
    :owner_(std::this_thread::get_id()),core_(std::move(c.core)),session_(core_,std::move(c.adapter)),normal_capacity_(c.inputs),cleanup_capacity_(c.cleanup_inputs){
    if(normal_capacity_==0 || normal_capacity_>1024 || cleanup_capacity_==0 || cleanup_capacity_>128 ||
       core_.config().capacity.orders>64 || core_.config().capacity.trades>256)throw std::invalid_argument("bounded local runtime profile required");
    if(c.quote){const auto& m=c.quote->market;const auto& s=c.quote->strategy;
        const auto contract=[](const Contract& v){return std::tie(v.instrument,v.product,v.exchange,v.version,v.day,v.low,v.high,v.tick_step,v.min_lots,v.max_lots,v.lot_step,v.synthetic_fixture);};
        if(m.scope!=core_.scope() || s.scope!=core_.scope() || s.strategy!=core_.config().strategy || contract(m.contract)!=contract(core_.config().contract) ||
           m.origin!=s.origin || m.source!=s.source || m.source_epoch!=s.market_epoch || m.max_age!=s.max_age)throw std::invalid_argument("matching runtime market and strategy profile required");
        market_.emplace(m);strategy_.emplace(s);session_.gate_.qualify_market(false);
    }
    trace_.reserve(normal_capacity_+cleanup_capacity_);
}
void LocalRuntimeRunner::require_owner() const {if(std::this_thread::get_id()!=owner_)throw std::logic_error("runtime owner required");}
Error LocalRuntimeRunner::withdraw_quotes(){
    Error first=Error::None;std::vector<OrderId> ids;ids.reserve(core_.state().orders.size());
    for(const auto& [id,order]:core_.state().orders){
        if(order.parts().held()==0 || order.cancel()==CancelState::Requested || order.cancel()==CancelState::Unknown)continue;
        if(order.execution()!=ExecutionState::Working && order.execution()!=ExecutionState::PartiallyFilled){if(first==Error::None)first=Error::Blocked;continue;}
        ids.push_back(id);
    }
    for(const auto id:ids){const auto result=session_.cancel(id,last_time_);if(first==Error::None && result.dispatch!=Error::None)first=result.dispatch;}
    return first;
}
Error LocalRuntimeRunner::drive_quotes(LocalRuntimeFrame& frame,LogicalTime at){
    if(!market_ || !quote_session_ || finished_)return Error::None;
    session_.gate_.qualify_market(market_->top(at).has_value());
    if(stopped_){frame.decision=QuoteDecision{Error::Halted,true,{}};return withdraw_quotes();}
    if(market_->top(at) && (session_.retained_reports()!=0 || session_.pending_outcomes()!=0)){
        frame.decision=QuoteDecision{Error::Blocked,false,{}};return Error::Blocked;
    }
    if(quote_generation_==std::numeric_limits<std::uint64_t>::max()){session_.gate_.close(LocalGateReason::CounterExhausted);return Error::IdExhausted;}
    frame.decision=strategy_->refresh(core_,*market_,*quote_session_,at,++quote_generation_);
    if(frame.decision->withdraw){const auto error=withdraw_quotes();return error==Error::None?frame.decision->error:error;}
    if(!frame.decision->target)return Error::Blocked;
    const auto top=market_->top(at);if(!top)return Error::Blocked;
    const auto mark=core_.step(ReferenceMark{top->bid+(top->ask-top->bid)/2},at);if(mark.error!=Error::None)return mark.error;
    frame.plan=plan_quotes(core_,*frame.decision->target,quote_generation_);if(frame.plan->error!=Error::None)return frame.plan->error;
    // One plan only: replacement awaits actual asynchronous terminal evidence and a subsequent pump.
    for(const auto& intent:frame.plan->intents){
        if(const auto* cancel=std::get_if<RequestCancel>(&intent)){const auto result=session_.cancel(cancel->order,at);if(result.dispatch!=Error::None)return result.dispatch;}
        else {const auto result=session_.submit(std::get<SubmitOrder>(intent),at);if(result.dispatch!=Error::None)return result.dispatch;}
    }return Error::None;
}
Error LocalRuntimeRunner::step(LocalRuntimeInput input,LogicalTime at){
    require_owner();if(fatal_)return Error::Halted;if(!trace_.empty() && at<last_time_)return Error::OutOfOrderTime;
    const bool cleanup=std::holds_alternative<KillRequested>(input) || std::holds_alternative<RuntimeCancel>(input) ||
        std::holds_alternative<RuntimePump>(input) || std::holds_alternative<RuntimeRetry>(input) || std::holds_alternative<RuntimeFinish>(input) ||
        std::holds_alternative<RuntimeQuoteTimer>(input) || std::holds_alternative<RuntimeFeedDisconnect>(input);
    if(market_ && (std::holds_alternative<SubmitOrder>(input) || std::holds_alternative<ReferenceMark>(input)))return Error::Blocked;
    if(const auto* snapshot=std::get_if<RuntimeMarketSnapshot>(&input)){
        const auto& m=snapshot->snapshot;if(m.source.size()>128 || m.scope.account.size()>128 || m.scope.contract.exchange.size()>128 || m.scope.contract.instrument.size()>128)return Error::InvalidInput;
    }
    if(const auto* submit=std::get_if<SubmitOrder>(&input)){
        if(submit->intent.account.size()>128 || submit->intent.instrument.size()>128 ||
           (submit->client_id && (submit->client_id->producer.size()>128 || submit->client_id->value.size()>128)) ||
           (submit->strategy && submit->strategy->value.size()>128))return Error::InvalidInput;
    }
    if(const auto* fill=std::get_if<RuntimeFill>(&input);fill && fill->trade_id.size()>128)return Error::InvalidInput;
    if(finished_ && !std::holds_alternative<RuntimePump>(input) && !std::holds_alternative<RuntimeRetry>(input) &&
       !std::holds_alternative<KillRequested>(input) && !std::holds_alternative<RuntimeFinish>(input))return Error::Halted;
    if(!stopped_ && normal_count_==normal_capacity_){
        stopped_=true;session_.gate_.close(LocalGateReason::Capacity);if(!cleanup)return Error::Capacity;
    }
    if(stopped_ && !cleanup)return Error::Halted;
    if(stopped_ && cleanup_count_==cleanup_capacity_)return Error::Capacity;
    LocalRuntimeFrame frame;frame.ordinal=trace_.size()+1;frame.at=at;frame.input=std::move(input);frame.cleanup=stopped_;
    trace_.push_back(std::move(frame));auto& f=trace_.back();last_time_=at;
    if(stopped_)++cleanup_count_;else ++normal_count_;
    try{
        session_.gate_.heartbeat();
        std::visit([&](const auto& value){using T=std::decay_t<decltype(value)>;
            if constexpr(std::is_same_v<T,RuntimeStart>){if(started_)f.error=Error::InvalidInput;else{session_.start();started_=true;}}
            else if constexpr(std::is_same_v<T,ReferenceMark>){f.core_result=core_.step(value,at);f.error=f.core_result->error;}
            else if constexpr(std::is_same_v<T,SubmitOrder>){const auto result=session_.submit(value,at);f.core_result=result.core;f.error=result.dispatch;}
            else if constexpr(std::is_same_v<T,RuntimeCancel>){const auto result=session_.cancel(value.order,at);f.core_result=result.core;f.error=result.dispatch;}
            else if constexpr(std::is_same_v<T,RuntimeFill>)f.error=session_.simulate_fill(value.order,value.trade_id,value.quantity,value.price);
            else if constexpr(std::is_same_v<T,KillRequested>){f.core_result=session_.kill(at);f.error=f.core_result->error;if(market_ && !finished_){const auto e=withdraw_quotes();if(f.error==Error::None)f.error=e;}}
            else if constexpr(std::is_same_v<T,RuntimePump>){if(market_)session_.gate_.qualify_market(market_->top(at).has_value());f.pump_result=session_.pump(at,value.budget);f.error=f.pump_result->error;if(f.error==Error::None){const auto e=drive_quotes(f,at);if(e!=Error::None)f.error=e;}}
            else if constexpr(std::is_same_v<T,RuntimeRetry>){f.error=session_.retry_reports(at);if(f.error==Error::None)f.error=drive_quotes(f,at);}
            else if constexpr(std::is_same_v<T,RuntimeFinish>){peer_query_=session_.finish();finished_=true;started_=true;f.error=peer_query_->error;}
            else if constexpr(std::is_same_v<T,RuntimeMarketSnapshot>){
                if(!market_)f.error=Error::InvalidInput;
                else {f.error=market_->update(value.snapshot,at);if(f.error==Error::None){quote_session_=value.session;f.error=drive_quotes(f,at);}
                    else if(!market_->top(at)){session_.gate_.qualify_market(false);const auto e=withdraw_quotes();if(e!=Error::None)f.error=e;}}
            }
            else if constexpr(std::is_same_v<T,RuntimeFeedDisconnect>){if(!market_)f.error=Error::InvalidInput;else {f.error=market_->disconnect(at);session_.gate_.qualify_market(false);const auto e=withdraw_quotes();if(e!=Error::None)f.error=e;}}
            else if constexpr(std::is_same_v<T,RuntimeFeedSubscription>){if(!market_)f.error=Error::InvalidInput;else {f.error=market_->begin_subscription(value.generation,at);if(!market_->top(at)){session_.gate_.qualify_market(false);const auto e=withdraw_quotes();if(e!=Error::None)f.error=e;}}}
            else if constexpr(std::is_same_v<T,RuntimeQuoteTimer>){if(!market_)f.error=Error::InvalidInput;else{quote_session_=value.session;f.error=market_->tick(at);if(f.error==Error::None)f.error=drive_quotes(f,at);}}
        },f.input);
        session_.gate_.heartbeat();
        if(market_)f.feed=market_->health(at);
        f.gate=session_.gate_snapshot();f.canonical=core_.canonical_state();f.completed=true;
    }catch(...){fatal_=true;stopped_=true;session_.gate_.close(LocalGateReason::Capacity);f.error=Error::InjectedFailure;throw;}
    return f.error;
}
}
