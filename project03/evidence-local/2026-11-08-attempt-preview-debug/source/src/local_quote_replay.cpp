#include "p03/local_quote_replay.hpp"
namespace p03 {
namespace {
bool same_session(const SessionPoint& a,const SessionPoint& b){return std::tie(a.civil_date,a.second,a.trading_day)==std::tie(b.civil_date,b.second,b.trading_day);}
bool same_trade(const TradeReport& a,const TradeReport& b){return a.trade==b.trade && a.stamp==b.stamp && a.channel==b.channel;}
bool same_fill(const LocalFillInput& a,const LocalFillInput& b){return std::tie(a.order,a.trade_id,a.quantity,a.price)==std::tie(b.order,b.trade_id,b.quantity,b.price);}
bool same_input(const LocalQuoteInput& a,const LocalQuoteInput& b){
    if(a.index()!=b.index())return false;
    return std::visit([&](const auto& x){using T=std::decay_t<decltype(x)>;const auto& y=std::get<T>(b);
        if constexpr(std::is_same_v<T,MarketPulse>){const auto& p=x.book;const auto& q=y.book;
            return same_session(x.session,y.session) && std::tie(p.scope,p.origin,p.source,p.epoch,p.sequence,p.received_at,p.bid,p.ask,p.bid_quantity,p.ask_quantity,p.healthy)==std::tie(q.scope,q.origin,q.source,q.epoch,q.sequence,q.received_at,q.bid,q.ask,q.bid_quantity,q.ask_quantity,q.healthy);}
        else if constexpr(std::is_same_v<T,LocalFillInput>)return same_fill(x,y);
        else if constexpr(std::is_same_v<T,LocalDeferredFillInput>)return same_fill(x.fill,y.fill);
        else if constexpr(std::is_same_v<T,LocalHeldReportInput>)return x.index==y.index;
        else if constexpr(std::is_same_v<T,LocalTradeQueryInput>)return std::tie(x.query,x.omit_last_trade,x.bypass_backpressure_for_test)==std::tie(y.query,y.omit_last_trade,y.bypass_backpressure_for_test);
        else if constexpr(std::is_same_v<T,LocalReportFailureInput>)return x.accepted_before_failure==y.accepted_before_failure;
        else return true;
    },a);
}
bool same_header(const EvidenceHeader& a,const EvidenceHeader& b){return std::tie(a.order,a.account,a.day,a.stamp)==std::tie(b.order,b.account,b.day,b.stamp);}
bool same_event(const BusinessEvent& a,const BusinessEvent& b){
    if(a.index()!=b.index())return false;
    return std::visit([&](const auto& x){using T=std::decay_t<decltype(x)>;const auto& y=std::get<T>(b);
        if constexpr(std::is_same_v<T,SubmitOrder>)return x.intent==y.intent && same_session(x.session,y.session) && x.client_id==y.client_id && x.strategy==y.strategy;
        else if constexpr(std::is_same_v<T,ReferenceMark>)return x.price==y.price;
        else if constexpr(std::is_same_v<T,RequestCancel>)return x.order==y.order;
        else if constexpr(std::is_same_v<T,TradeReport>)return same_trade(x,y);
        else if constexpr(std::is_same_v<T,WorkingReport>)return same_header(x.header,y.header);
        else if constexpr(std::is_same_v<T,TerminalReport>)return same_header(x.header,y.header) && x.proof.kind==y.proof.kind && x.proof.exact_quantity==y.proof.exact_quantity;
        else if constexpr(std::is_same_v<T,CancelReport>)return same_header(x.header,y.header) && x.action==y.action && x.state==y.state;
        else if constexpr(std::is_same_v<T,KillRequested>)return true;
        else return false; // Runner does not emit the remaining core event kinds.
    },a);
}
bool same_result(const StepResult& a,const StepResult& b){return std::tie(a.error,a.applied,a.command,a.core_sequence,a.ingress_sequence,a.admission,a.order,a.cancel)==std::tie(b.error,b.applied,b.command,b.core_sequence,b.ingress_sequence,b.admission,b.order,b.cancel);}
bool same_decision(const std::optional<QuoteDecision>& a,const std::optional<QuoteDecision>& b){
    if(a.has_value()!=b.has_value())return false;if(!a)return true;
    if(a->error!=b->error || a->withdraw!=b->withdraw || a->target.has_value()!=b->target.has_value())return false;if(!a->target)return true;
    const auto& x=*a->target;const auto& y=*b->target;
    const auto level=[](const std::optional<QuoteLevel>& p,const std::optional<QuoteLevel>& q){return p.has_value()==q.has_value() && (!p || (p->price==q->price && p->quantity==q->quantity));};
    return x.scope==y.scope && x.strategy==y.strategy && x.generation==y.generation && same_session(x.session,y.session) && level(x.buy,y.buy) && level(x.sell,y.sell);
}
bool same_peer(const std::optional<SimQuerySnapshot>& a,const std::optional<SimQuerySnapshot>& b){
    if(a.has_value()!=b.has_value())return false;if(!a)return true;const auto& x=*a;const auto& y=*b;
    if(std::tie(x.query,x.origin,x.model_epoch,x.revision,x.account,x.instrument,x.day,x.positions,x.frozen,x.available_budget,x.expected_orders,x.expected_trades,x.complete)!=
       std::tie(y.query,y.origin,y.model_epoch,y.revision,y.account,y.instrument,y.day,y.positions,y.frozen,y.available_budget,y.expected_orders,y.expected_trades,y.complete) || x.orders.size()!=y.orders.size() || x.trades.size()!=y.trades.size())return false;
    for(std::size_t i=0;i<x.orders.size();++i){const auto& p=x.orders[i];const auto& q=y.orders[i];
        if(p.request!=q.request || std::tie(p.venue_reference,p.status,p.filled,p.reserved_margin,p.reserved_fees,p.position_margin,p.paid_fees)!=std::tie(q.venue_reference,q.status,q.filled,q.reserved_margin,q.reserved_fees,q.position_margin,q.paid_fees))return false;
    }
    for(std::size_t i=0;i<x.trades.size();++i)if(!same_trade(x.trades[i],y.trades[i]))return false;return true;
}
LocalReplayMismatch compare_frame(const LocalQuoteFrame& actual,const LocalQuoteFrame& expected){
    if(actual.ordinal!=expected.ordinal || actual.at!=expected.at || !actual.completed || !expected.completed)return LocalReplayMismatch::FrameShape;
    if(!same_input(actual.input,expected.input))return LocalReplayMismatch::RecordedInput;
    if(actual.error!=expected.error)return LocalReplayMismatch::StepError;
            if(actual.canonical!=expected.canonical || actual.positions!=expected.positions || actual.mode!=expected.mode || actual.core_required!=expected.core_required)return LocalReplayMismatch::CoreState;
            if(!same_decision(actual.decision,expected.decision))return LocalReplayMismatch::StrategyDecision;
            if(actual.calls.size()!=expected.calls.size())return LocalReplayMismatch::CoreCalls;
            for(std::size_t j=0;j<actual.calls.size();++j)if(!same_event(actual.calls[j].input,expected.calls[j].input) || !same_result(actual.calls[j].result,expected.calls[j].result))return LocalReplayMismatch::CoreCalls;
            if(actual.orders.size()!=expected.orders.size())return LocalReplayMismatch::OrderViews;
            for(std::size_t j=0;j<actual.orders.size();++j){const auto& a=actual.orders[j];const auto& b=expected.orders[j];if(std::tie(a.id,a.intent,a.parts,a.execution)!=std::tie(b.id,b.intent,b.parts,b.execution))return LocalReplayMismatch::OrderViews;}
            if(!same_peer(actual.peer,expected.peer))return LocalReplayMismatch::PeerSnapshot;
            if(std::tie(actual.pending_reports,actual.pending_delivery,actual.query_blocked,actual.delivery_loss,actual.cleanup_lane,actual.query_aligned)!=std::tie(expected.pending_reports,expected.pending_delivery,expected.query_blocked,expected.delivery_loss,expected.cleanup_lane,expected.query_aligned))return LocalReplayMismatch::DeliveryState;
    return LocalReplayMismatch::None;
}

}
LocalReplayReview verify_local_quote_trace(const LocalQuoteRunnerConfig& config,std::span<const LocalQuoteFrame> frames){
    std::size_t matched=0;const auto fail=[&](Error error,LocalReplayMismatch why,std::size_t frame){return LocalReplayReview{error,why,frame,matched};};
    if(frames.empty())return fail(Error::InvalidInput,LocalReplayMismatch::EmptyTrace,0);
    try {
        LocalQuoteRunner replay(config);
        if(frames.size()>config.max_inputs+config.cleanup_inputs)return fail(Error::Capacity,LocalReplayMismatch::TraceCapacity,0);
        for(std::size_t i=0;i<frames.size();++i){const auto& expected=frames[i];
            if(expected.ordinal!=i+1 || !expected.completed)return fail(Error::InvalidInput,LocalReplayMismatch::FrameShape,i+1);
            const auto error=replay.step(expected.input,expected.at);
            if(error!=expected.error || replay.trace().size()!=i+1)return fail(Error::EvidenceConflict,LocalReplayMismatch::StepError,i+1);
            const auto& actual=replay.trace().back();
            const auto mismatch=compare_frame(actual,expected);
            if(mismatch!=LocalReplayMismatch::None)return fail(Error::EvidenceConflict,mismatch,i+1);
            ++matched;
        }
        return {Error::None,LocalReplayMismatch::None,0,matched};
    }catch(const std::invalid_argument&){return fail(Error::InvalidInput,LocalReplayMismatch::InvalidConfiguration,matched+1);}
     catch(const std::bad_alloc&){return fail(Error::Capacity,LocalReplayMismatch::ReplayException,matched+1);}
     catch(...){return fail(Error::InjectedFailure,LocalReplayMismatch::ReplayException,matched+1);}
}
LocalExperimentReview verify_local_quote_experiment(const LocalQuoteRunnerConfig& config,
    std::span<const LocalQuoteAttempt> attempts,std::span<const LocalQuoteFrame> frames){
    std::size_t matched=0,frame_count=0;
    const auto fail=[&](Error error,LocalReplayMismatch why,std::size_t attempt){return LocalExperimentReview{error,why,attempt,matched,frame_count};};
    if(attempts.empty())return fail(Error::InvalidInput,LocalReplayMismatch::EmptyTrace,0);
    if(attempts.size()>4096)return fail(Error::Capacity,LocalReplayMismatch::TraceCapacity,0);
    try {
        LocalQuoteRunner replay(config);
        if(frames.size()>config.max_inputs+config.cleanup_inputs)return fail(Error::Capacity,LocalReplayMismatch::TraceCapacity,0);
        for(std::size_t i=0;i<attempts.size();++i){const auto& a=attempts[i];
            if(a.ordinal!=i+1 || a.frames_before!=frame_count || a.frames_after<a.frames_before || a.frames_after-a.frames_before>1 || a.frames_after>frames.size())return fail(Error::InvalidInput,LocalReplayMismatch::AttemptShape,i+1);
            if(replay.step(a.input,a.at)!=a.error)return fail(Error::EvidenceConflict,LocalReplayMismatch::AttemptResult,i+1);
            if(replay.trace().size()!=a.frames_after)return fail(Error::EvidenceConflict,LocalReplayMismatch::AttemptShape,i+1);
            if(replay.core().canonical_state()!=a.canonical)return fail(Error::EvidenceConflict,LocalReplayMismatch::CoreState,i+1);
            if(replay.stopped()!=a.stopped || replay.cleanup_exhausted()!=a.cleanup_exhausted)return fail(Error::EvidenceConflict,LocalReplayMismatch::RunnerState,i+1);
            if(a.frames_after>frame_count){
                const auto mismatch=compare_frame(replay.trace().back(),frames[frame_count]);
                if(mismatch!=LocalReplayMismatch::None)return fail(Error::EvidenceConflict,mismatch,i+1);++frame_count;
            }
            ++matched;
        }
        if(frame_count!=frames.size())return fail(Error::InvalidInput,LocalReplayMismatch::AttemptShape,attempts.size());
        return {Error::None,LocalReplayMismatch::None,0,matched,frame_count};
    }catch(const std::invalid_argument&){return fail(Error::InvalidInput,LocalReplayMismatch::InvalidConfiguration,matched+1);}
     catch(const std::bad_alloc&){return fail(Error::Capacity,LocalReplayMismatch::ReplayException,matched+1);}
     catch(...){return fail(Error::InjectedFailure,LocalReplayMismatch::ReplayException,matched+1);}
}
}
