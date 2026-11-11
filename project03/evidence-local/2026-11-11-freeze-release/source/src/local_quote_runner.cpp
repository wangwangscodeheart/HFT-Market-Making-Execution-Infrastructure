#include "p03/local_quote_runner.hpp"
#include "p03/local_query_review.hpp"
#include <algorithm>
namespace p03 {
LocalQuoteRunner::LocalQuoteRunner(LocalQuoteRunnerConfig c)
    :core_(c.core),peer_(c.peer),strategy_(c.strategy),peer_config_(c.peer),inbox_(c.peer.max_trades+2*c.core.capacity.orders),input_capacity_(c.max_inputs),call_capacity_(c.max_core_calls),
     per_input_bound_(4*c.core.capacity.orders+c.peer.max_trades+8),cleanup_input_capacity_(c.cleanup_inputs),cleanup_call_capacity_(c.cleanup_core_calls){
    market_max_age_=c.strategy.max_age;
    if(c.core.trade_evidence_mode!=TradeEvidenceMode::Simulator || c.strategy.scope!=core_.scope() ||
       c.strategy.strategy!=c.core.strategy || c.peer.account!=core_.scope().account ||
       c.peer.rules.instrument!=core_.scope().contract.instrument || c.peer.rules.exchange!=core_.scope().contract.exchange ||
       c.peer.rules.day!=core_.scope().day || c.peer.initial_positions!=c.core.anchor.booked ||
       c.core.capacity.orders==0 || c.core.capacity.orders>64 || input_capacity_==0 || input_capacity_>1024 ||
       call_capacity_<per_input_bound_ || call_capacity_>65536 || cleanup_input_capacity_==0 || cleanup_input_capacity_>128 ||
       cleanup_call_capacity_<per_input_bound_ || cleanup_call_capacity_>65536)throw std::invalid_argument("explicit bounded local runner configuration required");
    if(c.market){
        const auto& m=*c.market;
        if(m.scope!=c.strategy.scope || m.contract.key()!=c.core.contract.key() || m.contract.low!=c.core.contract.low || m.contract.high!=c.core.contract.high || m.contract.tick_step!=c.core.contract.tick_step || m.origin!=c.strategy.origin ||
           m.source!=c.strategy.source || m.source_epoch!=c.strategy.market_epoch || m.max_age!=c.strategy.max_age)
            throw std::invalid_argument("market profile must match runner and strategy");
        market_state_.emplace(m);
    }
    trace_.reserve(input_capacity_+cleanup_input_capacity_);
    held_.reserve(c.peer.max_trades);
}
std::size_t LocalQuoteRunner::pending_reports() const {
    std::size_t pending=0;
    for(const auto& report:held_)if(core_.state().executions.classify(report.trade)!=DedupResult::Identical)++pending;
    return pending;
}
StepResult LocalQuoteRunner::call(LocalQuoteFrame& f,const BusinessEvent& event){
    if(f.calls.size()>=per_input_bound_)throw std::logic_error("local runner call bound");
    f.calls.push_back({event,{}}); // Allocate/copy payload before entering core.
    const auto result=core_.step(f.calls.back().input,f.at);f.calls.back().result=result;
    if(f.cleanup_lane)++cleanup_calls_;else ++call_count_;return result;
}
Error LocalQuoteRunner::reports(LocalQuoteFrame& f,const SimReply& reply){
    if(reply.reports.empty())return reply.error;
    const bool waiting=inbox_.size()!=0;const auto accepted=inbox_.enqueue(reply.reports);
    if(accepted!=Error::None){query_blocked_=true;delivery_loss_=true;return accepted;}
    return waiting?Error::Blocked:drain(f);
}
Error LocalQuoteRunner::drain(LocalQuoteFrame& f){
    while(inbox_.size()!=0){
        if(f.calls.size()>=per_input_bound_)return Error::Capacity;
        if(report_failure_ && *report_failure_==0){core_.fail_next_commit_for_test();report_failure_.reset();}
        const auto result=call(f,inbox_.front());
        if(result.error!=Error::None && result.error!=Error::Duplicate)return result.error;
        inbox_.acknowledge();if(report_failure_ && *report_failure_>0)--*report_failure_;
    }
    return Error::None;
}
Error LocalQuoteRunner::cancel(LocalQuoteFrame& f,OrderId id){
    const auto canceled=call(f,RequestCancel{id});
    if(canceled.error!=Error::None || !canceled.cancel)return canceled.error==Error::None?Error::Blocked:canceled.error;
    auto reply=peer_.cancel(id,*canceled.cancel);bool omitted=false;
    if(cancel_terminal_gap_){
        const auto terminal=std::find_if(reply.reports.begin(),reply.reports.end(),[](const BusinessEvent& event){return std::holds_alternative<TerminalReport>(event);});
        if(terminal!=reply.reports.end()){reply.reports.erase(terminal);cancel_terminal_gap_=false;query_blocked_=true;omitted=true;}
    }
    const auto error=reports(f,reply);return omitted && error==Error::None?Error::Blocked:error;
}
Error LocalQuoteRunner::withdraw(LocalQuoteFrame& f){
    std::vector<OrderId> ids;ids.reserve(core_.state().orders.size());
    for(const auto& [id,order]:core_.state().orders)if((order.parts().possible>0 || order.parts().unresolved>0) &&
        order.cancel()!=CancelState::Requested && order.cancel()!=CancelState::Unknown)ids.push_back(id);
    Error first_error=Error::None;
    for(const auto id:ids){const auto error=cancel(f,id);if(error!=Error::None && first_error==Error::None)first_error=error;}
    return first_error;
}
Error LocalQuoteRunner::market(LocalQuoteFrame& f,const MarketPulse& pulse){
    if(inbox_.size()!=0 || query_blocked_){f.decision=QuoteDecision{Error::Blocked,true,{}};return Error::Blocked;}
    if(pending_reports()!=0){
        f.decision=QuoteDecision{Error::Blocked,true,{}};
        const auto error=withdraw(f);return error==Error::None?Error::Blocked:error;
    }
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
    if(fatal_)return Error::Halted;
    const bool cleanup_allowed=std::holds_alternative<StopLocalQuotes>(input) || std::holds_alternative<LocalRetryEvidenceInput>(input) ||
        std::holds_alternative<LocalHeldReportInput>(input) || std::holds_alternative<LocalTradeQueryInput>(input) || std::holds_alternative<LocalQuoteTimer>(input);
    if(stopped_ && !cleanup_allowed)return Error::Halted;
    if(!trace_.empty() && at<last_time_)return Error::OutOfOrderTime;
    if(!stopped_ && (normal_inputs_>=input_capacity_ || call_count_>call_capacity_-per_input_bound_)){
        stopped_=true;if(!cleanup_allowed)return Error::Capacity;
    }
    const bool cleanup=stopped_;
    if(cleanup && (cleanup_inputs_>=cleanup_input_capacity_ || cleanup_calls_>cleanup_call_capacity_-per_input_bound_)){
        cleanup_exhausted_=true;return Error::Capacity;
    }
    LocalQuoteFrame frame;frame.ordinal=static_cast<std::uint64_t>(trace_.size()+1);frame.at=at;frame.input=std::move(input);
    frame.cleanup_lane=cleanup;
    frame.calls.reserve(per_input_bound_);trace_.push_back(std::move(frame));auto& f=trace_.back();last_time_=at;
    if(cleanup)++cleanup_inputs_;else ++normal_inputs_;
    try{
        if(const auto* snapshot=std::get_if<LocalMarketSnapshotInput>(&f.input)){
            if(!market_state_)f.error=Error::InvalidInput;
            else {
                const auto updated=market_state_->update(snapshot->snapshot,at);
                if(updated==Error::None){
                    const auto top=market_state_->top(at);
                    if(top)f.error=market(f,MarketPulse{*top,snapshot->session});
                    else f.error=Error::Blocked;
                }else {
                    f.error=updated;
                    if(!market_state_->top(at)){f.decision=QuoteDecision{updated,true,{}};const auto e=withdraw(f);if(e!=Error::None)f.error=e;}
                }
            }
        }
        else if(std::holds_alternative<LocalMarketSubscriptionInput>(f.input) || std::holds_alternative<LocalMarketDisconnectInput>(f.input) || std::holds_alternative<LocalMarketLossInput>(f.input)){
            if(!market_state_)f.error=Error::InvalidInput;
            else {
                if(const auto* subscription=std::get_if<LocalMarketSubscriptionInput>(&f.input))f.error=market_state_->begin_subscription(subscription->generation,at);
                else if(std::holds_alternative<LocalMarketDisconnectInput>(f.input))f.error=market_state_->disconnect(at);
                else f.error=market_state_->local_loss(at);
                if(!market_state_->top(at)){f.decision=QuoteDecision{Error::Blocked,true,{}};const auto e=withdraw(f);if(e!=Error::None)f.error=e;}
            }
        }
        else if(const auto* pulse=std::get_if<MarketPulse>(&f.input))f.error=market_state_?Error::InvalidInput:market(f,*pulse);
        else if(const auto* fill=std::get_if<LocalFillInput>(&f.input))f.error=reports(f,peer_.execute_fill(fill->order,fill->trade_id,fill->quantity,fill->price));
        else if(const auto* deferred=std::get_if<LocalDeferredFillInput>(&f.input)){
            const auto& deferred_fill=deferred->fill;
            const auto reply=peer_.execute_fill(deferred_fill.order,deferred_fill.trade_id,deferred_fill.quantity,deferred_fill.price);f.error=reply.error;
            if(reply.error==Error::None){
                if(held_.size()>=peer_config_.max_trades)throw std::logic_error("held report bound");
                for(const auto& event:reply.reports)held_.push_back(std::get<TradeReport>(event));
            }
        }
        else if(const auto* delivery=std::get_if<LocalHeldReportInput>(&f.input)){
            if(delivery->index>=held_.size())f.error=Error::InvalidInput;
            else {
                const auto& report=held_[delivery->index];
                const bool duplicate=core_.state().executions.classify(report.trade)==DedupResult::Identical;
                f.error=inbox_.contains_exact(report)?Error::Blocked:
                    reports(f,SimReply{Error::None,0,{BusinessEvent{report}}});
                if(f.error==Error::None && duplicate)f.error=Error::Duplicate;
            }
        }
        else if(const auto* query=std::get_if<LocalTradeQueryInput>(&f.input)){
            if(inbox_.size()!=0 && !query->bypass_backpressure_for_test){
                query_blocked_=true;f.error=Error::Blocked;
            }else{
            auto result=peer_.query(query->query);
            if(query->omit_last_trade && result.snapshot && !result.snapshot->trades.empty())result.snapshot->trades.pop_back();
            auto query_delivery=plan_local_query_delivery(result,query->query,peer_config_,core_);f.error=query_delivery.review.error;
            query_blocked_=true;
            if(query_delivery.review.status==LocalQueryStatus::Complete){
                auto events=std::move(query_delivery.events);
                if(events.empty())f.error=inbox_.size()==0?Error::None:Error::Blocked;
                else f.error=reports(f,SimReply{Error::None,result.snapshot->revision,std::move(events)});
                if(f.error==Error::None){
                    const auto alignment=compare_local_query_with_core(result,query->query,peer_config_,core_);
                    f.query_aligned=alignment.status==LocalQueryStatus::Complete;
                    if(!*f.query_aligned)f.error=alignment.error;
                    else {query_blocked_=delivery_loss_;if(delivery_loss_)f.error=Error::Blocked;}
                }
            }
            }
        }
        else if(std::holds_alternative<LocalRetryEvidenceInput>(f.input))f.error=drain(f);
        else if(const auto* failure=std::get_if<LocalReportFailureInput>(&f.input)){
            if(failure->accepted_before_failure>per_input_bound_)f.error=Error::InvalidInput;
            else report_failure_=failure->accepted_before_failure;
        }
        else if(std::holds_alternative<LocalCancelTerminalGapInput>(f.input))cancel_terminal_gap_=true;
        else if(std::holds_alternative<LocalQuoteTimer>(f.input)){
            if(market_state_)market_state_->tick(at);
            if((market_state_ && !market_state_->top(at)) || (last_market_at_ && f.at-*last_market_at_>market_max_age_)){
                const auto error=withdraw(f);f.error=error==Error::None?Error::StaleMark:error;
            }
        }
        else {f.error=call(f,KillRequested{}).error;if(f.error==Error::None)f.error=withdraw(f);}
        if(market_state_)f.feed=market_state_->health(at);
        f.canonical=core_.canonical_state();const auto q=peer_.query({core_.config().instance,f.ordinal});f.peer=q.snapshot;
        f.mode=core_.state().mode;f.positions=core_.state().positions.booked();
        for(const auto& [id,order]:core_.state().orders)f.orders.push_back({id,order.intent(),order.parts(),order.execution()});
        f.pending_reports=pending_reports();
        f.pending_delivery=inbox_.size();f.query_blocked=query_blocked_;
        f.delivery_loss=delivery_loss_;
        f.core_required=core_.state().funds.view().local_required;
        if(!core_.invariant() || !peer_.invariant() || q.error!=Error::None || !q.snapshot)throw std::logic_error("local runner invariant");
        f.completed=true;
    }catch(...){f.error=Error::InjectedFailure;stopped_=true;fatal_=true;throw;}
    return f.error;
}
} // namespace p03
