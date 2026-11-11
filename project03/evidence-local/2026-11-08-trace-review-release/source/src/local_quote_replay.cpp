#include "p03/local_quote_replay.hpp"
namespace p03 {
namespace {
bool same_session(const SessionPoint& a,const SessionPoint& b){return std::tie(a.civil_date,a.second,a.trading_day)==std::tie(b.civil_date,b.second,b.trading_day);}
bool same_trade(const TradeReport& a,const TradeReport& b){return a.trade==b.trade && a.stamp==b.stamp && a.channel==b.channel;}
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
            if(actual.canonical!=expected.canonical || actual.positions!=expected.positions || actual.mode!=expected.mode || actual.core_required!=expected.core_required)return fail(Error::EvidenceConflict,LocalReplayMismatch::CoreState,i+1);
            if(!same_decision(actual.decision,expected.decision))return fail(Error::EvidenceConflict,LocalReplayMismatch::StrategyDecision,i+1);
            if(actual.calls.size()!=expected.calls.size())return fail(Error::EvidenceConflict,LocalReplayMismatch::CoreCalls,i+1);
            for(std::size_t j=0;j<actual.calls.size();++j)if(!same_event(actual.calls[j].input,expected.calls[j].input) || !same_result(actual.calls[j].result,expected.calls[j].result))return fail(Error::EvidenceConflict,LocalReplayMismatch::CoreCalls,i+1);
            if(actual.orders.size()!=expected.orders.size())return fail(Error::EvidenceConflict,LocalReplayMismatch::OrderViews,i+1);
            for(std::size_t j=0;j<actual.orders.size();++j){const auto& a=actual.orders[j];const auto& b=expected.orders[j];if(std::tie(a.id,a.intent,a.parts,a.execution)!=std::tie(b.id,b.intent,b.parts,b.execution))return fail(Error::EvidenceConflict,LocalReplayMismatch::OrderViews,i+1);}
            if(!same_peer(actual.peer,expected.peer))return fail(Error::EvidenceConflict,LocalReplayMismatch::PeerSnapshot,i+1);
            if(std::tie(actual.pending_reports,actual.pending_delivery,actual.query_blocked,actual.delivery_loss,actual.cleanup_lane,actual.query_aligned)!=std::tie(expected.pending_reports,expected.pending_delivery,expected.query_blocked,expected.delivery_loss,expected.cleanup_lane,expected.query_aligned))return fail(Error::EvidenceConflict,LocalReplayMismatch::DeliveryState,i+1);
            ++matched;
        }
        return {Error::None,LocalReplayMismatch::None,0,matched};
    }catch(const std::invalid_argument&){return fail(Error::InvalidInput,LocalReplayMismatch::InvalidConfiguration,matched+1);}
     catch(const std::bad_alloc&){return fail(Error::Capacity,LocalReplayMismatch::ReplayException,matched+1);}
     catch(...){return fail(Error::InjectedFailure,LocalReplayMismatch::ReplayException,matched+1);}
}
}
