#include "p03/core.hpp"
#include <algorithm>
#include <new>
#include <iomanip>
#include <sstream>
#include <type_traits>

namespace p03 {
namespace {
template<class... T> struct Visitor : T... { using T::operator()...; };
template<class... T> Visitor(T...)->Visitor<T...>;
bool text_ok(const std::string& s) { return !s.empty() && s.size()<=128; }
Error trust_error(const EvidenceStamp& s,TradeEvidenceMode mode) {
    if(s.origin==EvidenceOrigin::ExternalUnconfigured) return Error::Blocked;
    if(s.origin!=trade_origin(mode)) return Error::UntrustedEvidence;
    if(!text_ok(s.source) || !s.source.starts_with(trade_source_prefix(mode)) || s.epoch==0 ||
       (s.comparable && !s.sequence)) return Error::InvalidInput;
    return Error::None;
}
bool trade_text_ok(const Trade& t) {
    return text_ok(t.key.account) && text_ok(t.key.exchange) && text_ok(t.key.scope) &&
        text_ok(t.key.trade_id) && text_ok(t.instrument);
}
Error observation_error(EvidenceOrigin origin,const ObservationMetadata& m) {
    if(origin==EvidenceOrigin::ExternalUnconfigured) return Error::Blocked;
    if(origin!=EvidenceOrigin::LocalFixture) return Error::UntrustedEvidence;
    if(!text_ok(m.source) || !m.source.starts_with("fixture.") || m.profile!="p03.fixture.v1" ||
       !text_ok(m.scope) || m.currency!="FIXTURE_UNIT" || m.request_begin>m.response_end) return Error::InvalidInput;
    return Error::None;
}
} // namespace

std::string_view admission_name(InputAdmission admission) noexcept {
    switch(admission){
        case InputAdmission::RejectedInput:return "RejectedInput";
        case InputAdmission::BusinessInput:return "BusinessInput";
        case InputAdmission::RejectedEvidenceFact:return "RejectedEvidenceFact";
    }
    return "InvalidAdmission";
}

CoreLoop::CoreLoop(CoreConfig c) : config_(std::move(c)),
    scope_{config_.anchor.account,config_.contract.key(),config_.anchor.day,config_.anchor.epoch},
    risk_(config_.risk), state_(config_),
    order_ids_(config_.instance,config_.next_order), command_ids_(config_.instance),
    cancel_ids_(config_.instance), reservation_ids_(config_.instance,2), anchor_resource_{config_.instance,1} {
    config_.contract.check_configuration();
    if(!config_.strategy.valid())throw std::invalid_argument("explicit strategy identity required");
    if(config_.trade_evidence_mode!=TradeEvidenceMode::Fixture && config_.trade_evidence_mode!=TradeEvidenceMode::Simulator)
        throw std::invalid_argument("explicit local trade evidence mode required");
    if(config_.contract.day!=config_.anchor.day || config_.contract.instrument!=config_.anchor.instrument ||
       !config_.contract.synthetic_fixture) throw std::invalid_argument("M1A requires an explicit synthetic fixture profile");
    const auto& cap=config_.capacity;
    for(const auto n:{cap.orders,cap.trades,cap.quarantine,cap.observations,cap.evidence_streams_per_order,cap.cancel_actions_per_order})
        if(n==0 || n>4096) throw std::invalid_argument("M1A capacity must be in [1,4096]");
    recompute(state_);
    if(!invariant()) throw std::logic_error("invalid initial state");
}
Availability CoreLoop::capability(FutureOperation op) noexcept {
    switch(op) {
        case FutureOperation::CtpConnect:
        case FutureOperation::ReconcileExternal: return Availability::Blocked;
        default: return Availability::NotImplemented;
    }
}
Error CoreLoop::header_error(const CoreState& s,const EvidenceHeader& h) const {
    const auto trust=trust_error(h.stamp,config_.trade_evidence_mode);
    if(trust!=Error::None) return trust;
    const auto it=s.orders.find(h.order);
    if(it==s.orders.end()) return Error::UnknownOrder;
    if(h.account!=it->second.intent().account) return Error::WrongIdentity;
    if(h.day!=it->second.intent().day) return Error::WrongTradingDay;
    return Error::None;
}
CoreLoop::InputCheck CoreLoop::validate_input(const BusinessEvent& event) const {
    return std::visit([&](const auto& x)->InputCheck {
        using T=std::decay_t<decltype(x)>;
        InputCheck check;
        if constexpr(requires{x.header;}) {
            check.critical=true;check.order=x.header.order;
            if constexpr(std::is_same_v<T,CumulativeReport>)check.quantity=x.quantity;
            if constexpr(std::is_same_v<T,TerminalReport>)check.quantity=x.proof.exact_quantity.value_or(-1);
            check.error=header_error(state_,x.header);
            if(check.error!=Error::None)return check;
            const auto& order=state_.orders.at(x.header.order);
            if constexpr(std::is_same_v<T,CumulativeReport>) {
                if(x.quantity<0 || x.quantity>order.intent().quantity)check.error=Error::InvalidInput;
            }else if constexpr(std::is_same_v<T,TerminalReport>) {
                const auto f=x.proof.exact_quantity;const auto kind=x.proof.kind;
                if((kind!=TerminalKind::Filled && kind!=TerminalKind::Canceled && kind!=TerminalKind::Rejected && kind!=TerminalKind::NotSent) ||
                   (f && (*f<0 || *f>order.intent().quantity)) ||
                   (kind==TerminalKind::Filled && (!f || *f!=order.intent().quantity)) ||
                   ((kind==TerminalKind::Rejected || kind==TerminalKind::NotSent) && (!f || *f!=0)))check.error=Error::InvalidInput;
            }else if constexpr(std::is_same_v<T,CancelReport>) {
                if(!order.has_cancel_action(x.action))check.error=Error::WrongIdentity;
                else if(x.state!=CancelState::Succeeded && x.state!=CancelState::Rejected && x.state!=CancelState::Unknown)check.error=Error::InvalidInput;
            }
        }else if constexpr(std::is_same_v<T,TradeReport>) {
            const auto& t=x.trade;check.critical=true;check.order=t.order;check.quantity=t.quantity;
            check.error=trust_error(x.stamp,config_.trade_evidence_mode);
            if(check.error==Error::None && ((x.channel!=Channel::Push && x.channel!=Channel::Query) ||
               !trade_text_ok(t) || !t.order.valid() || t.quantity<=0 || t.quantity>max_input_lots ||
               t.price<=0 || t.price>max_input_ticks))check.error=Error::InvalidInput;
            if(check.error!=Error::None)return check;
            const auto dedup=state_.executions.classify(t);
            if(dedup==DedupResult::Identical)return check;
            if(dedup==DedupResult::Conflicting){
                check.error=Error::EvidenceConflict;check.related_order=state_.executions.records().at(t.key).trade.order;return check;
            }
            const auto found=state_.orders.find(t.order);
            if(found==state_.orders.end())check.error=Error::UnknownOrder;
            else if(t.key.account!=found->second.intent().account || !scope_.contract.matches(t.key.exchange,t.instrument) ||
                    t.key.scope!=economic_trade_scope(config_.trade_evidence_mode) || t.side!=found->second.intent().side || t.offset!=found->second.intent().offset)
                check.error=Error::WrongIdentity;
            else if(t.key.day!=found->second.intent().day)check.error=Error::WrongTradingDay;
        }else if constexpr(std::is_same_v<T,SubmitOrder>) {
            if((x.strategy && !x.strategy->valid()) || (x.client_id && !x.client_id->valid()))check.error=Error::InvalidInput;
            else if(x.strategy && *x.strategy!=config_.strategy)check.error=Error::WrongIdentity;
            else if(!text_ok(x.intent.account) || x.intent.account!=scope_.account)check.error=Error::WrongIdentity;
            else if((x.intent.side!=Side::Buy && x.intent.side!=Side::Sell) ||
                    (x.intent.offset!=Offset::Open && x.intent.offset!=Offset::CloseToday && x.intent.offset!=Offset::CloseYesterday))check.error=Error::InvalidInput;
            else if(!x.session.civil_date.valid() || !x.session.trading_day.valid() || x.session.second<0 || x.session.second>=86400)check.error=Error::InvalidInput;
            else check.error=config_.contract.validate(x.intent);
        }else if constexpr(std::is_same_v<T,ReferenceMark>) {
            if(x.price<=0 || x.price>max_input_ticks)check.error=Error::InvalidInput;
        }else if constexpr(std::is_same_v<T,RequestCancel> || std::is_same_v<T,OrderTimeout>) {
            if(!state_.orders.contains(x.order))check.error=Error::UnknownOrder;
        }else if constexpr(std::is_same_v<T,DayBoundary>) {
            if(!x.next_day.valid() || x.next_day<=scope_.day)check.error=Error::WrongTradingDay;
        }else if constexpr(std::is_same_v<T,ObservePosition> || std::is_same_v<T,ObserveSettlement>) {
            if(config_.trade_evidence_mode==TradeEvidenceMode::Simulator){check.error=Error::Blocked;return check;}
            const auto& o=x.observation;check.error=observation_error(o.origin,o.metadata);
            if(check.error!=Error::None)return check;
            if(o.account!=scope_.account)check.error=Error::WrongIdentity;
            if constexpr(std::is_same_v<T,ObservePosition>) {
                if(o.instrument!=scope_.contract.instrument || o.shape!="p03.fixture.four-buckets.v1")check.error=Error::WrongIdentity;
                if(check.error!=Error::None)return check;
                if(!o.day.valid() || o.observation_id==0)check.error=Error::InvalidInput;
                for(const auto q:o.reported)if(q<0 || q>max_input_lots)check.error=Error::InvalidInput;
            }else{
                if(check.error!=Error::None)return check;
                if(!o.day.valid() || o.record_id==0 || o.reported_total<0 || o.reported_total>4*max_input_lots)check.error=Error::InvalidInput;
            }
        }else if constexpr(std::is_same_v<T,ObserveFunds>) {
            if(config_.trade_evidence_mode==TradeEvidenceMode::Simulator){check.error=Error::Blocked;return check;}
            check.error=state_.funds.validate(x.observation);
            // Duplicates and conflicts are semantic business evidence, not malformed envelopes.
            if(check.error==Error::Duplicate || check.error==Error::EvidenceConflict)check.error=Error::None;
        }else if constexpr(std::is_same_v<T,RequestFutureOperation>) {
            if(x.operation<FutureOperation::SendOrder || x.operation>FutureOperation::Reactivate)check.error=Error::InvalidInput;
        }
        return check;
    },event);
}
void CoreLoop::quarantine(CoreState& s,Error error,OrderId order,Lots q,std::optional<Trade> trade) {
    s.mode=CoreMode::ConflictHalted;
    s.unknown_exposure=true;
    const auto it=s.orders.find(order);
    if(it!=s.orders.end()) it->second.mark_conflict();
    if(s.quarantine.size()>=config_.capacity.quarantine) {
        if(s.evidence_loss_count<std::numeric_limits<std::uint64_t>::max()) ++s.evidence_loss_count;
        s.mode=CoreMode::CapacityHalted; return;
    }
    if(trade && !trade_text_ok(*trade)) trade.reset(); // Oversize raw input stays with the caller.
    s.quarantine.push_back({error,order,q,std::move(trade),ingress_sequence_,sequence_});
}
void CoreLoop::recompute(CoreState& s) const {
    std::array<Lots,4> frozen{};
    for(const auto& [id,o]:s.orders) {
        (void)id;
        if(o.intent().offset==Offset::Open) continue;
        const auto b=static_cast<std::size_t>(PositionLedger::affected_bucket(o.intent().side,o.intent().offset));
        frozen[b]=checked_add(frozen[b],o.parts().held());
    }
    s.positions.replace_freezes(frozen);
    s.funds.replace_resources(risk_.resources(s.positions,s.orders,s.executions,anchor_resource_));
    const auto rv=risk_.calculate(s.positions,s.orders,s.executions,false);
    const auto deficit=risk_.check(rv,s.funds.view());
    if(deficit!=Error::None && deficit!=Error::Halted) {
        s.risk_deficit=true;
        if(s.mode!=CoreMode::CapacityHalted) s.mode=CoreMode::ConflictHalted;
    }
    for(const auto& [id,o]:s.orders) {
        (void)id;
        if(o.consistency()==Consistency::Conflict && s.mode!=CoreMode::CapacityHalted) s.mode=CoreMode::ConflictHalted;
    }
}
StepResult CoreLoop::step(const BusinessEvent& event,LogicalTime now) {
    StepResult result;
    if(ingress_sequence_==std::numeric_limits<std::uint64_t>::max()){
        state_.mode=CoreMode::ConflictHalted;state_.unknown_exposure=true;result.error=Error::IdExhausted;return result;
    }
    result.ingress_sequence=++ingress_sequence_;
    if(now<last_time_) { result.error=Error::OutOfOrderTime; return result; }
    try {
        const auto check=validate_input(event);
        if(check.error!=Error::None && !check.critical){result.error=check.error;return result;}
        if(sequence_==std::numeric_limits<std::uint64_t>::max()) {
            state_.mode=CoreMode::ConflictHalted; result.error=Error::IdExhausted; return result;
        }
        const auto command=command_ids_.take();
        if(!command) { state_.mode=CoreMode::ConflictHalted; result.error=Error::IdExhausted; return result; }
        result.command=*command; result.core_sequence=++sequence_; last_time_=now;
        result.admission=check.error==Error::None?InputAdmission::BusinessInput:InputAdmission::RejectedEvidenceFact;
        auto candidate=state_;
        auto orders=order_ids_; auto cancels=cancel_ids_; auto reservations=reservation_ids_;
        if(check.error!=Error::None){
            if(check.related_order.valid())candidate.orders.at(check.related_order).mark_conflict();
            std::optional<Trade> trade;
            if(const auto* report=std::get_if<TradeReport>(&event))trade=report->trade;
            quarantine(candidate,check.error,check.order,check.quantity,std::move(trade));
            result.error=check.error;result.applied=true;
        }else result.error=apply(candidate,event,now,result,orders,cancels,reservations);
        if(!result.applied) return result;
        recompute(candidate);
        if(!invariant(candidate)) throw std::logic_error("business invariant violation before commit");
        if(fail_next_commit_) {
            fail_next_commit_=false; result.error=Error::InjectedFailure; result.applied=false;
            result.order.reset(); result.cancel.reset(); return result;
        }
        static_assert(std::is_nothrow_swappable_v<CoreState>);
        using std::swap; swap(state_,candidate);
        order_ids_=orders; cancel_ids_=cancels; reservation_ids_=reservations;
    } catch(const std::bad_alloc&) {
        state_.mode=CoreMode::CapacityHalted; state_.unknown_exposure=true;
        result.error=Error::Capacity; result.applied=false; result.order.reset(); result.cancel.reset();
    } catch(const std::overflow_error&) {
        state_.mode=CoreMode::ConflictHalted; state_.unknown_exposure=true;
        result.error=Error::ArithmeticOverflow; result.applied=false; result.order.reset(); result.cancel.reset();
    }
    return result;
}
Error CoreLoop::apply(CoreState& s,const BusinessEvent& event,LogicalTime now,StepResult& out,
    IdSequence<OrderTag>& orders,IdSequence<CancelTag>& cancels,IdSequence<ReservationTag>& reservations) {
    auto validate_header=[&](const EvidenceHeader& h,Lots q=0) {
        const auto error=header_error(s,h);
        if(error!=Error::None) { quarantine(s,error,h.order,q); out.applied=true; }
        return error;
    };
    return std::visit(Visitor{
        [&](const SubmitOrder& x)->Error {
            if(x.client_id){
                const auto existing=s.client_orders.find({config_.strategy,*x.client_id});
                if(existing!=s.client_orders.end()){
                    out.order=existing->second;
                    return s.orders.at(existing->second).intent()==x.intent ? Error::Duplicate : Error::EvidenceConflict;
                }
            }
            if(s.mode!=CoreMode::Ready) return Error::Halted;
            const auto& i=x.intent;
            if(!text_ok(i.account) || i.account!=scope_.account) return Error::WrongIdentity;
            if(i.side!=Side::Buy && i.side!=Side::Sell) return Error::InvalidInput;
            if(i.offset!=Offset::Open && i.offset!=Offset::CloseToday && i.offset!=Offset::CloseYesterday) return Error::InvalidInput;
            const auto rule=config_.contract.validate(i); if(rule!=Error::None) return rule;
            const auto session=config_.calendar.validate(x.session,config_.contract.day); if(session!=Error::None) return session;
            if(i.quantity>risk_.limits().max_order) return Error::QuantityRule;
            if(!s.mark || now-s.mark->second>risk_.limits().max_mark_age) return Error::StaleMark;
            const auto distance=i.price>s.mark->first ? i.price-s.mark->first : s.mark->first-i.price;
            if(distance>risk_.limits().max_deviation) return Error::PriceRule;
            if(s.orders.size()>=config_.capacity.orders) return Error::Capacity;
            auto& times=s.order_times;
            std::erase_if(times,[&](LogicalTime t){return now-t>=risk_.limits().rate_window;});
            if(times.size()>=risk_.limits().order_rate) return Error::RateLimit;
            if(risk_.crosses(i,s.orders)) return Error::SelfTrade;
            if(i.offset!=Offset::Open && s.positions.available(PositionLedger::affected_bucket(i.side,i.offset))<i.quantity)
                return Error::InsufficientClose;
            const auto id=orders.take(); const auto reservation=reservations.take();
            if(!id || !reservation) return Error::IdExhausted;
            s.orders.emplace(*id,Order{*id,*reservation,i,config_.capacity.cancel_actions_per_order,x.client_id,config_.strategy});
            if(x.client_id && !s.client_orders.emplace(ClientRequestKey{config_.strategy,*x.client_id},*id).second)
                throw std::logic_error("duplicate accepted client identity");
            recompute(s);
            const auto check=risk_.check(risk_.calculate(s.positions,s.orders,s.executions,s.unknown_exposure),s.funds.view());
            if(check!=Error::None) return check;
            times.push_back(now); out.order=id; out.applied=true; return Error::None;
        },
        [&](const ReferenceMark& x)->Error {
            if(x.price<=0 || x.price>max_input_ticks) return Error::InvalidInput;
            s.mark=std::pair{x.price,now}; out.applied=true; return Error::None;
        },
        [&](const WorkingReport& x)->Error {
            const auto e=validate_header(x.header); if(e!=Error::None) return e;
            const auto r=s.orders.at(x.header.order).observe_working();
            if(r!=Error::None) quarantine(s,r,x.header.order,0);
            out.applied=true; return r;
        },
        [&](const CumulativeReport& x)->Error {
            const auto e=validate_header(x.header,x.quantity); if(e!=Error::None) return e;
            auto& o=s.orders.at(x.header.order);
            if(x.header.stamp.comparable && !o.has_stream(x.header.stamp) &&
               o.stream_count()>=config_.capacity.evidence_streams_per_order) {
                quarantine(s,Error::Capacity,x.header.order,x.quantity); out.applied=true; return Error::Capacity;
            }
            const auto r=o.observe_cumulative(x.quantity,x.header.stamp);
            if(r!=Error::None) quarantine(s,r,x.header.order,x.quantity);
            out.applied=true; return r;
        },
        [&](const TerminalReport& x)->Error {
            const auto e=validate_header(x.header,x.proof.exact_quantity.value_or(-1)); if(e!=Error::None) return e;
            const auto r=s.orders.at(x.header.order).observe_terminal(x.proof);
            if(r!=Error::None) quarantine(s,r,x.header.order,x.proof.exact_quantity.value_or(-1));
            out.applied=true; return r;
        },
        [&](const TradeReport& x)->Error {
            const auto& t=x.trade;
            auto e=trust_error(x.stamp,config_.trade_evidence_mode);
            if(e==Error::None && ((x.channel!=Channel::Push && x.channel!=Channel::Query) ||
                !trade_text_ok(t) || !t.order.valid() || t.quantity<=0 ||
                t.quantity>max_input_lots || t.price<=0 || t.price>max_input_ticks)) e=Error::InvalidInput;
            if(e!=Error::None) { quarantine(s,e,t.order,t.quantity,t); out.applied=true; return e; }
            const auto dedup=s.executions.classify(t);
            if(dedup==DedupResult::Identical) return Error::Duplicate;
            if(dedup==DedupResult::Conflicting) {
                const auto original=s.executions.records().at(t.key).trade.order;
                s.orders.at(original).mark_conflict();
                quarantine(s,Error::EvidenceConflict,t.order,t.quantity,t); out.applied=true; return Error::EvidenceConflict;
            }
            const auto found=s.orders.find(t.order);
            if(found==s.orders.end()) e=Error::UnknownOrder;
            else if(t.key.account!=found->second.intent().account ||
                !scope_.contract.matches(t.key.exchange,t.instrument) || t.key.scope!=economic_trade_scope(config_.trade_evidence_mode) ||
                t.side!=found->second.intent().side || t.offset!=found->second.intent().offset) e=Error::WrongIdentity;
            else if(t.key.day!=found->second.intent().day) e=Error::WrongTradingDay;
            if(e!=Error::None) { quarantine(s,e,t.order,t.quantity,t); out.applied=true; return e; }
            if(s.executions.records().size()>=config_.capacity.trades) {
                quarantine(s,Error::Capacity,t.order,t.quantity,t); out.applied=true; return Error::Capacity;
            }
            auto& o=found->second;
            if(t.quantity>o.intent().quantity-o.booked()) {
                s.executions.insert(t,LedgerDisposition::Exception); o.mark_conflict();
                quarantine(s,Error::EvidenceConflict,t.order,t.quantity,t); out.applied=true; return Error::EvidenceConflict;
            }
            s.executions.insert(t,LedgerDisposition::Normal);
            o.book_trade(t.quantity); s.positions.book(t.side,t.offset,t.quantity);
            if((t.side==Side::Buy && t.price>o.intent().price) || (t.side==Side::Sell && t.price<o.intent().price))
                o.mark_conflict();
            if(o.consistency()==Consistency::Conflict) {
                quarantine(s,Error::EvidenceConflict,t.order,t.quantity,t); e=Error::EvidenceConflict;
            }
            out.applied=true; return e;
        },
        [&](const RequestCancel& x)->Error {
            const auto it=s.orders.find(x.order); if(it==s.orders.end()) return Error::UnknownOrder;
            const auto action=cancels.take(); if(!action) return Error::IdExhausted;
            const auto e=it->second.request_cancel(*action);
            if(e==Error::None) { out.applied=true; out.cancel=action; }
            return e;
        },
        [&](const CancelReport& x)->Error {
            const auto e=validate_header(x.header); if(e!=Error::None) return e;
            const auto r=s.orders.at(x.header.order).observe_cancel(x.action,x.state);
            if(r!=Error::None && r!=Error::Duplicate) quarantine(s,r,x.header.order,0);
            if(r==Error::None && x.state==CancelState::Unknown && s.mode==CoreMode::Ready)
                s.mode=CoreMode::ConflictHalted;
            out.applied=r!=Error::Duplicate; return r;
        },
        [&](const OrderTimeout& x)->Error {
            const auto it=s.orders.find(x.order); if(it==s.orders.end()) return Error::UnknownOrder;
            it->second.mark_unknown(); if(s.mode==CoreMode::Ready) s.mode=CoreMode::ConflictHalted;
            out.applied=true; return Error::None;
        },
        [&](const KillRequested&)->Error {
            if(s.mode==CoreMode::Ready) s.mode=CoreMode::Killed;
            out.applied=true; return Error::None;
        },
        [&](const DayBoundary& x)->Error {
            if(!x.next_day.valid() || x.next_day<=config_.anchor.day) return Error::WrongTradingDay;
            s.proposed_day=x.next_day;
            bool unresolved=s.unknown_exposure || s.risk_deficit;
            for(const auto& [id,o]:s.orders) {
                (void)id; unresolved=unresolved || o.parts().held()!=0 || o.consistency()!=Consistency::InSync;
            }
            s.mode=unresolved ? CoreMode::BoundaryHalted : CoreMode::AwaitDayEvidence;
            out.applied=true; return Error::Blocked;
        },
        [&](const ObservePosition& x)->Error {
            const auto& o=x.observation;
            const auto meta=observation_error(o.origin,o.metadata); if(meta!=Error::None) return meta;
            if(o.account!=config_.anchor.account || o.instrument!=config_.anchor.instrument ||
               o.shape!="p03.fixture.four-buckets.v1") return Error::WrongIdentity;
            if(!o.day.valid() || o.observation_id==0) return Error::InvalidInput;
            for(auto q:o.reported) if(q<0 || q>max_input_lots) return Error::InvalidInput;
            if(s.external_positions.size()>=config_.capacity.observations) return Error::Capacity;
            s.external_positions.push_back(o); out.applied=true; return Error::None;
        },
        [&](const ObserveSettlement& x)->Error {
            const auto& o=x.observation;
            const auto meta=observation_error(o.origin,o.metadata); if(meta!=Error::None) return meta;
            if(o.account!=config_.anchor.account) return Error::WrongIdentity;
            if(!o.day.valid() || o.record_id==0 || o.reported_total<0 || o.reported_total>4*max_input_lots) return Error::InvalidInput;
            if(s.settlements.size()>=config_.capacity.observations) return Error::Capacity;
            s.settlements.push_back(o); out.applied=true; return Error::None;
        },
        [&](const ObserveFunds& x)->Error {
            const auto e=s.funds.observe(x.observation); out.applied=e==Error::None;
            if(e==Error::EvidenceConflict) { quarantine(s,e,{},0); out.applied=true; }
            return e;
        },
        [&](const RequestFutureOperation& x)->Error {
            return capability(x.operation)==Availability::Blocked ? Error::Blocked : Error::NotImplemented;
        }
    },event);
}
RiskView CoreLoop::risk_view() const {
    auto v=risk_.calculate(state_.positions,state_.orders,state_.executions,state_.unknown_exposure);
    v.funds_deficit=!state_.funds.view().sufficient(); return v;
}
bool CoreLoop::invariant(const CoreState& s) const {
    if(s.client_orders.size()>s.orders.size())return false;
    for(const auto& [key,id]:s.client_orders){
        const auto found=s.orders.find(id);
        if(!key.valid() || found==s.orders.end() || !found->second.client_id() ||
           *found->second.client_id()!=key.client || found->second.strategy()!=key.strategy)return false;
    }
    if(s.positions.anchor().account!=scope_.account || s.positions.anchor().instrument!=scope_.contract.instrument ||
       s.positions.anchor().day!=scope_.day || s.positions.anchor().epoch!=scope_.position_epoch) return false;
    if(s.orders.size()>config_.capacity.orders || s.executions.records().size()>config_.capacity.trades ||
       s.quarantine.size()>config_.capacity.quarantine || s.external_positions.size()>config_.capacity.observations ||
       s.settlements.size()>config_.capacity.observations || s.order_times.size()>risk_.limits().order_rate) return false;
    PositionLedger expected(config_.anchor); std::array<Lots,4> freezes{};
    for(const auto& [key,r]:s.executions.records()) {
        if(key!=r.trade.key || !s.orders.contains(r.trade.order)) return false;
        const auto& intent=s.orders.at(r.trade.order).intent();
        if(key.account!=scope_.account || key.day!=scope_.day ||
           !scope_.contract.matches(key.exchange,r.trade.instrument) ||
           r.trade.side!=intent.side || r.trade.offset!=intent.offset) return false;
        if(r.disposition==LedgerDisposition::Normal) expected.book(r.trade.side,r.trade.offset,r.trade.quantity);
    }
    for(const auto& [id,o]:s.orders) {
        if(id!=o.id() || !o.invariant() || o.stream_count()>config_.capacity.evidence_streams_per_order ||
           o.booked()!=s.executions.normal_quantity(id) || o.send()!=SendState::NotImplemented || o.external_identity() ||
           o.cancel_action_count()>config_.capacity.cancel_actions_per_order) return false;
        if(id.instance!=config_.instance || o.intent().account!=scope_.account ||
           o.intent().instrument!=scope_.contract.instrument || o.intent().day!=scope_.day || o.strategy()!=config_.strategy) return false;
        if(o.client_id()){
            const auto found=s.client_orders.find({o.strategy(),*o.client_id()});
            if(found==s.client_orders.end() || found->second!=id)return false;
        }
        if(o.intent().offset!=Offset::Open) {
            const auto b=static_cast<std::size_t>(PositionLedger::affected_bucket(o.intent().side,o.intent().offset));
            freezes[b]=checked_add(freezes[b],o.parts().held());
        }
        if(o.consistency()==Consistency::Conflict && s.mode==CoreMode::Ready) return false;
    }
    if(expected.booked()!=s.positions.booked() || freezes!=s.positions.frozen() || s.positions.anchor()!=config_.anchor) return false;
    const auto resources=risk_.resources(s.positions,s.orders,s.executions,anchor_resource_);
    if(resources.size()!=s.funds.resources().size()) return false;
    for(const auto& r:resources) {
        const auto it=s.funds.resources().find(r.id);
        if(it==s.funds.resources().end() || it->second!=r) return false;
    }
    if(s.mode==CoreMode::Ready && (s.unknown_exposure || s.risk_deficit ||
       risk_.check(risk_.calculate(s.positions,s.orders,s.executions,false),s.funds.view())!=Error::None)) return false;
    return true;
}
bool CoreLoop::invariant() const { return invariant(state_); }
std::string CoreLoop::canonical_state() const {
    // Deterministic summary; not a durable snapshot or cryptographic audit record.
    std::ostringstream o;
    o<<"domain.v6|"<<config_.instance<<'|'<<config_.contract.instrument<<'|'<<config_.contract.version<<'|'
     <<config_.calendar.version()<<'|'<<config_.anchor.day.ymd<<'|'<<config_.anchor.epoch<<'|'
     <<sequence_<<'|'<<last_time_<<'|'<<static_cast<int>(state_.mode)<<'|'<<state_.unknown_exposure<<'|'<<state_.risk_deficit;
    o<<"|scope:"<<std::quoted(scope_.account)<<':'<<std::quoted(scope_.contract.exchange)<<':'
     <<std::quoted(scope_.contract.instrument)<<':'<<scope_.day.ymd<<':'<<scope_.position_epoch;
    o<<"|strategy:"<<std::quoted(config_.strategy.value);
    o<<"|trade-mode:"<<static_cast<int>(config_.trade_evidence_mode);
    for(auto q:state_.positions.booked()) o<<'|'<<q;
    for(auto q:state_.positions.frozen()) o<<'|'<<q;
    if(state_.mark) o<<"|mark:"<<state_.mark->first<<':'<<state_.mark->second;
    if(state_.proposed_day) o<<"|day:"<<state_.proposed_day->ymd;
    for(const auto& [id,r]:state_.orders) {
        (void)id; o<<"|order:"<<r.canonical_state();
    }
    for(const auto& [key,id]:state_.client_orders)
        o<<"|client-index:"<<std::quoted(key.strategy.value)<<':'<<std::quoted(key.client.producer)<<':'
         <<std::quoted(key.client.value)<<':'<<id.instance<<':'<<id.serial;
    for(const auto& [key,r]:state_.executions.records())
        o<<"|trade:"<<std::quoted(key.account)<<':'<<key.day.ymd<<':'<<std::quoted(key.exchange)<<':'<<std::quoted(key.scope)<<':'<<std::quoted(key.trade_id)<<':'
         <<r.trade.order.serial<<':'<<r.trade.quantity<<':'<<r.trade.price<<':'<<static_cast<int>(r.disposition);
    for(const auto& [id,r]:state_.funds.resources()) o<<"|fund:"<<id.serial<<':'<<r.order_margin<<':'<<r.position_margin<<':'<<r.charges;
    const auto f=state_.funds.view();
    o<<"|budget:"<<f.local_required<<':'<<f.proven_cover<<':'<<f.external_budget<<':'<<f.has_external;
    o<<"|quarantine:"<<state_.quarantine.size()<<':'<<state_.evidence_loss_count;
    for(const auto& q:state_.quarantine) o<<':'<<static_cast<int>(q.reason)<<','<<q.order.serial<<','<<q.reported_quantity
        <<','<<q.ingress_sequence<<','<<q.core_sequence;
    for(const auto t:state_.order_times) o<<"|admitted:"<<t;
    if(const auto& obs=state_.funds.observation()) {
        o<<"|funds-observation:"<<obs->snapshot_id<<':'<<obs->external_available;
        for(const auto& proof:obs->coverage) o<<':'<<proof.resource.serial<<','<<static_cast<int>(proof.status)<<','<<proof.amount
            <<','<<std::quoted(proof.external_deduction_id);
    }
    for(const auto& obs:state_.external_positions) {
        o<<"|position-observation:"<<obs.day.ymd<<':'<<obs.observation_id;
        for(auto q:obs.reported) o<<':'<<q;
    }
    for(const auto& obs:state_.settlements) o<<"|settlement:"<<obs.day.ymd<<':'<<obs.record_id<<':'<<obs.reported_total;
    o<<"|ids:"<<order_ids_.next_serial()<<':'<<order_ids_.exhausted()<<':'<<command_ids_.next_serial()<<':'<<command_ids_.exhausted()
     <<':'<<cancel_ids_.next_serial()<<':'<<cancel_ids_.exhausted()<<':'<<reservation_ids_.next_serial()<<':'<<reservation_ids_.exhausted();
    return o.str();
}
} // namespace p03
