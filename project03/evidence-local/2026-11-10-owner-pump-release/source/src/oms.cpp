#include "p03/oms.hpp"
#include <algorithm>
#include <iomanip>
#include <sstream>

namespace p03 {
Order::Order(OrderId id, ReservationId rid, OrderIntent i,std::size_t cancel_capacity,std::optional<ClientOrderId> client_id,StrategyId strategy)
    : id_(id), reservation_(rid), intent_(std::move(i)),client_id_(std::move(client_id)),strategy_(std::move(strategy)),cancel_capacity_(cancel_capacity) {
    if(!id.valid() || !rid.valid() || id.instance!=rid.instance || intent_.quantity<=0 || intent_.quantity>max_input_lots ||
       !intent_.day.valid() || intent_.account.empty() || intent_.account.size()>128 ||
       intent_.instrument.empty() || intent_.instrument.size()>128 || intent_.price<=0 ||
       intent_.price>max_input_ticks || (intent_.side!=Side::Buy && intent_.side!=Side::Sell) ||
       (intent_.offset!=Offset::Open && intent_.offset!=Offset::CloseToday && intent_.offset!=Offset::CloseYesterday) ||
       cancel_capacity_==0 || cancel_capacity_>4096 || (client_id_ && !client_id_->valid()) || !strategy_.valid())
        throw std::invalid_argument("invalid order construction");
}
QuantityParts Order::parts() const {
    const Lots e=std::max(booked_,lower_bound_);
    QuantityParts p{booked_, e-booked_,0,0,0};
    if(conflict_) p.possible=intent_.quantity-e;
    else if(final_) { p.suspense=*final_-booked_; p.released=intent_.quantity-*final_; }
    else if(no_future_) p.unresolved=intent_.quantity-e;
    else p.possible=intent_.quantity-e;
    return p;
}
Consistency Order::consistency() const {
    if(conflict_) return Consistency::Conflict;
    const auto p=parts();
    if(p.suspense>0) return Consistency::MissingTrades;
    if(unknown_ || p.unresolved>0) return Consistency::Unreconciled;
    for(const auto& [id,state]:actions_) { (void)id; if(state==CancelState::Unknown) return Consistency::Unreconciled; }
    return Consistency::InSync;
}
bool Order::invariant() const {
    const auto p=parts();
    return id_.valid() && reservation_.valid() && strategy_.valid() && (!client_id_ || client_id_->valid()) && booked_>=0 && booked_<=intent_.quantity &&
        lower_bound_>=0 && lower_bound_<=intent_.quantity && p.suspense>=0 && p.possible>=0 &&
        p.released>=0 && p.unresolved>=0 && p.total()==intent_.quantity && actions_.size()<=cancel_capacity_;
}
Error Order::observe_cumulative(Lots q,const EvidenceStamp& s) {
    if(s.origin!=EvidenceOrigin::LocalFixture && s.origin!=EvidenceOrigin::LocalSimulator) return Error::UntrustedEvidence;
    if(q<0 || q>intent_.quantity || s.source.empty() || s.source.size()>128 || s.epoch==0 ||
       (s.comparable && !s.sequence)) return Error::InvalidInput;
    if(s.comparable) {
        const auto key=std::pair{s.source,s.epoch};
        const auto found=versions_.find(key);
        if(found!=versions_.end()) {
            const auto& prev=found->second;
            if((*s.sequence>prev.seq && q<prev.quantity) ||
               (*s.sequence==prev.seq && q!=prev.quantity) ||
               (*s.sequence<prev.seq && q>prev.quantity)) conflict_=true;
        }
        if(found==versions_.end() || *s.sequence>found->second.seq) versions_[key]={*s.sequence,q};
    }
    lower_bound_=std::max(lower_bound_,q);
    if(!no_future_ && lower_bound_>0) execution_=lower_bound_==intent_.quantity ? ExecutionState::Filled : ExecutionState::PartiallyFilled;
    if(final_ && lower_bound_>*final_) conflict_=true;
    return conflict_ ? Error::EvidenceConflict : Error::None;
}
Error Order::observe_terminal(TerminalProof proof) {
    const auto f=proof.exact_quantity;
    if(proof.kind!=TerminalKind::Filled && proof.kind!=TerminalKind::Canceled &&
       proof.kind!=TerminalKind::Rejected && proof.kind!=TerminalKind::NotSent) return Error::InvalidInput;
    if((f && (*f<0 || *f>intent_.quantity)) ||
       (proof.kind==TerminalKind::Filled && (!f || *f!=intent_.quantity)) ||
       ((proof.kind==TerminalKind::Rejected || proof.kind==TerminalKind::NotSent) && (!f || *f!=0)))
        return Error::InvalidInput;
    if(f && (booked_>*f || lower_bound_>*f || (final_ && *f!=*final_))) conflict_=true;
    if(accepted_ && (proof.kind==TerminalKind::Rejected || proof.kind==TerminalKind::NotSent)) conflict_=true;
    if(f) { if(!final_) final_=f; lower_bound_=std::max(lower_bound_,*f); }
    if(!terminal_kind_) terminal_kind_=proof.kind;
    no_future_=true;
    switch(proof.kind) {
        case TerminalKind::Filled: execution_=ExecutionState::Filled; break;
        case TerminalKind::Canceled: execution_=ExecutionState::Canceled; break;
        case TerminalKind::Rejected: execution_=ExecutionState::Rejected; break;
        case TerminalKind::NotSent: execution_=ExecutionState::NotSent; break;
    }
    if(final_ && *final_==intent_.quantity && !conflict_) execution_=ExecutionState::Filled;
    return conflict_ ? Error::EvidenceConflict : Error::None;
}
Error Order::observe_working() noexcept {
    accepted_=true;
    if(terminal_kind_==TerminalKind::Rejected || terminal_kind_==TerminalKind::NotSent) {
        conflict_=true; return Error::EvidenceConflict;
    }
    if(!no_future_ && booked_==0 && lower_bound_==0) execution_=ExecutionState::Working;
    return Error::None;
}
void Order::book_trade(Lots q) {
    if(q<=0 || q>intent_.quantity-booked_) throw std::invalid_argument("book normal trade only within Q");
    booked_+=q;
    if(final_ && booked_>*final_) conflict_=true;
    if(!no_future_) execution_=booked_==intent_.quantity ? ExecutionState::Filled : ExecutionState::PartiallyFilled;
    if(booked_==intent_.quantity && !conflict_) { final_=booked_; no_future_=true; }
}
Error Order::request_cancel(CancelActionId id) {
    if(!id.valid() || id.instance!=id_.instance) return Error::InvalidInput;
    if(action_ && cancel_==CancelState::Requested) return Error::Duplicate;
    if(actions_.size()>=cancel_capacity_) return Error::Capacity;
    if(parts().possible==0 && parts().unresolved==0) return Error::InvalidInput;
    actions_.emplace(id,CancelState::Requested); action_=id; cancel_=CancelState::Requested; return Error::None;
}
Error Order::observe_cancel(CancelActionId id,CancelState result) {
    const auto found=actions_.find(id);
    if(found==actions_.end()) return Error::WrongIdentity;
    if(result!=CancelState::Succeeded && result!=CancelState::Rejected && result!=CancelState::Unknown)
        return Error::InvalidInput;
    const auto previous=found->second;
    if(previous==result) return Error::Duplicate;
    if(previous==CancelState::Succeeded || previous==CancelState::Rejected) {
        if(result==CancelState::Unknown) return Error::Duplicate;
        conflict_=true; return Error::EvidenceConflict;
    }
    found->second=result;
    if(action_==id) cancel_=result;
    return Error::None; // Cancel success is not final quantity evidence.
}
std::string Order::canonical_state() const {
    std::ostringstream o;
    o<<id_.instance<<':'<<id_.serial<<':'<<reservation_.serial<<':'<<std::quoted(intent_.account)<<':'
     <<std::quoted(intent_.instrument)<<':'<<intent_.day.ymd<<':'<<static_cast<int>(intent_.side)<<':'
     <<static_cast<int>(intent_.offset)<<':'<<intent_.price<<':'<<intent_.quantity<<':'<<booked_<<':'<<lower_bound_
     <<':'<<final_.value_or(-1)<<':'<<no_future_<<':'<<conflict_<<':'<<unknown_<<':'<<accepted_<<':'
     <<(terminal_kind_ ? static_cast<int>(*terminal_kind_) : -1)<<':'<<static_cast<int>(execution_)<<':'<<static_cast<int>(cancel_);
    for(const auto& [key,v]:versions_) o<<";version:"<<std::quoted(key.first)<<':'<<key.second<<':'<<v.seq<<':'<<v.quantity;
    for(const auto& [id,state]:actions_) o<<";action:"<<id.serial<<':'<<static_cast<int>(state);
    if(action_) o<<";current:"<<action_->serial;
    if(client_id_)o<<";client:"<<std::quoted(client_id_->producer)<<':'<<std::quoted(client_id_->value);
    o<<";strategy:"<<std::quoted(strategy_.value);
    return o.str();
}
} // namespace p03
