#include "p03/local_adapter_session.hpp"
namespace p03 {
namespace {
LocalGateConfig gate_config(const CoreLoop& core){return {core.config().instance,{"local.adapter.session"},core.scope(),core.config().strategy,
    {1},{{{"local.adapter.memory"},{1}},{1}},64};}
}
LocalAdapterSession::LocalAdapterSession(CoreLoop& core,LocalAdapterConfig c)
    :lease_(core),core_(core),owner_(std::this_thread::get_id()),config_(std::move(c)),gate_(gate_config(core)),commands_(config_.commands),cancels_(config_.cancels),fills_(config_.fills),reports_(config_.reports,gate_){
    const auto& p=config_.peer;const auto& rules=core.config().contract;
    if(core.config().trade_evidence_mode!=TradeEvidenceMode::Simulator || !core.state().orders.empty() || core.state().mode!=CoreMode::Ready ||
       config_.commands>64 || config_.reports>256 || config_.cancels>64 || config_.cancel_requests==0 || config_.cancel_requests>512 ||
       config_.fills>256 || config_.fill_requests==0 || config_.fill_requests>1024 ||
       config_.ttl_ms==0 || config_.ttl_ms>60000 || p.account!=core.scope().account || p.rules.instrument!=rules.instrument ||
       p.rules.exchange!=rules.exchange || p.rules.day!=rules.day || p.rules.low!=rules.low || p.rules.high!=rules.high ||
       p.rules.tick_step!=rules.tick_step || p.rules.min_lots!=rules.min_lots || p.rules.max_lots!=rules.max_lots || p.rules.lot_step!=rules.lot_step ||
       p.initial_positions!=core.config().anchor.booked || p.initial_budget!=core.config().risk.local_funds ||
       p.opening_margin_per_lot!=core.config().risk.opening_margin_per_lot || p.fee_per_lot!=core.config().risk.fee_per_lot)
        throw std::invalid_argument("matching explicit local adapter profile required");
}
LocalAdapterSession::CoreLease::CoreLease(CoreLoop& core):core_(core){
    if(core_.local_adapter_attached_.exchange(true))throw std::logic_error("core already has a local adapter session");
}
LocalAdapterSession::CoreLease::~CoreLease(){core_.local_adapter_attached_.store(false);}
void LocalAdapterSession::require_owner() const {if(std::this_thread::get_id()!=owner_)throw std::logic_error("core owner thread required");}
LocalAdapterSession::~LocalAdapterSession(){
    gate_.close(LocalGateReason::Kill);commands_.close();cancels_.close();fills_.close();wake_.close();
    if(worker_.valid()){try{worker_.get();}catch(...){}} // No core access or fabricated release during shutdown.
}
LocalDispatchSubmission LocalAdapterSession::submit(const SubmitOrder& input,LogicalTime at){
    require_owner();LocalDispatchSubmission result;
    if(closed_ || !gate_.snapshot().open){result.dispatch=Error::Halted;return result;}
    if(at>std::numeric_limits<LogicalTime>::max()-config_.ttl_ms){result.dispatch=Error::IdExhausted;return result;}
    result.core=core_.step(input,at);
    if(result.core.error!=Error::None || !result.core.applied || !result.core.order){result.dispatch=result.core.error;return result;}
    try{
        const auto& order=core_.state().orders.at(*result.core.order);
        Job job{{},SimOrderRequest{order.id(),order.intent(),core_.config().strategy,order.client_id()},at,std::chrono::steady_clock::now()};
        DispatchAuthorizationFact fact{{result.core.command.instance,result.core.command.serial},result.core.command,order.id(),order.reservation(),
            {"local.adapter.session"},core_.scope(),gate_.snapshot().epoch,{{{"local.adapter.memory"},{1}},{1}},at+config_.ttl_ms,core_.config().strategy};
        const auto authorization=gate_.enqueue(std::move(fact));result.dispatch=authorization.first;result.permit=authorization.second;
        if(!result.permit)return result;
        job.permit=*result.permit;
        if(commands_.try_push(std::move(job))!=QueueResult::Success){
            gate_.close(LocalGateReason::Capacity);gate_.abort_before_call(*result.permit);result.dispatch=Error::Capacity;
        }else wake_.notify();
    }catch(...){gate_.close(LocalGateReason::Capacity);throw;}
    return result;
}
void LocalAdapterSession::start(){require_owner();if(closed_ || worker_.valid())throw std::logic_error("local adapter start once before finish");
    try{worker_=std::async(std::launch::async,[this]{return work();});}catch(...){gate_.close(LocalGateReason::Capacity);throw;}
}
LocalCancelSubmission LocalAdapterSession::cancel(OrderId id,LogicalTime at){
    require_owner();LocalCancelSubmission result;
    if(closed_){result.dispatch=Error::Halted;return result;}
    if(cancel_requests_>=config_.cancel_requests || cancels_.snapshot().size==config_.cancels){result.dispatch=Error::Capacity;return result;}
    const auto it=core_.state().orders.find(id);
    if(it==core_.state().orders.end()){result.dispatch=Error::UnknownOrder;return result;}
    if(it->second.execution()==ExecutionState::Created){result.dispatch=Error::Blocked;return result;}
    result.core=core_.step(RequestCancel{id},at);result.dispatch=result.core.error;
    if(result.core.error!=Error::None || !result.core.applied || !result.core.cancel)return result;
    ++cancel_requests_;
    if(cancels_.try_push(CancelJob{id,*result.core.cancel})!=QueueResult::Success){gate_.close(LocalGateReason::Capacity);result.dispatch=Error::Capacity;}
    else wake_.notify();
    return result;
}
Error LocalAdapterSession::simulate_fill(OrderId id,std::string trade_id,Lots quantity,Ticks price){
    require_owner();if(closed_)return Error::Halted;
    if(trade_id.empty() || trade_id.size()>128 || quantity<=0 || quantity>max_input_lots || price<=0 || price>max_input_ticks)return Error::InvalidInput;
    const auto found=core_.state().orders.find(id);if(found==core_.state().orders.end())return Error::UnknownOrder;
    if(found->second.execution()==ExecutionState::Created)return Error::Blocked;
    if(fill_requests_>=config_.fill_requests)return Error::Capacity;
    const auto result=fills_.try_push(FillJob{id,std::move(trade_id),quantity,price});
    if(result!=QueueResult::Success)return Error::Capacity;
    ++fill_requests_;wake_.notify();return Error::None;
}
StepResult LocalAdapterSession::kill(LogicalTime at){require_owner();gate_.close(LocalGateReason::Kill);return core_.step(KillRequested{},at);}
QueuePop<LocalAdapterOutcome> LocalAdapterSession::receive(){
    require_owner();if(retained_)throw std::logic_error("retained evidence requires explicit retry before raw receive");return reports_.try_receive();
}
std::size_t LocalAdapterSession::retained_reports() const {
    require_owner();return retained_?retained_->reply.reports.size()-report_index_:0;
}
Error LocalAdapterSession::apply_retained(LogicalTime at){
    try{
        while(report_index_<retained_->reply.reports.size()){
            const auto result=core_.step(retained_->reply.reports[report_index_],at);
            if(result.error!=Error::None && result.error!=Error::Duplicate){gate_.close(LocalGateReason::EvidenceFailure);return result.error;}
            ++report_index_;
        }
        const auto error=retained_->error;retained_.reset();report_index_=0;return error;
    }catch(...){gate_.close(LocalGateReason::EvidenceFailure);throw;}
}
Error LocalAdapterSession::process_reports(LogicalTime at){
    require_owner();if(retained_)return Error::Blocked;
    auto outcome=reports_.try_receive();if(outcome.result!=QueueResult::Success)return Error::None;
    retained_.emplace(std::move(*outcome.value));report_index_=0;return apply_retained(at);
}
Error LocalAdapterSession::retry_reports(LogicalTime at){require_owner();return retained_?apply_retained(at):Error::None;}
StepResult LocalAdapterSession::sync_faults(LogicalTime at){
    require_owner();const auto observed=gate_.snapshot();
    const RuntimeFaultState fault{observed.reasons&std::uint32_t{126},observed.latch.value,observed.loss.value};
    if(fault.reasons==0 || fault==core_.state().runtime_fault)return {};
    return core_.step(LocalRuntimeFault{core_.scope(),core_.config().instance,fault},at);
}
const SimQueryResult& LocalAdapterSession::finish(){
    require_owner();if(final_)return *final_;if(!worker_.valid())start();closed_=true;commands_.close();cancels_.close();fills_.close();wake_.close();
    try{final_=worker_.get();}catch(...){gate_.close(LocalGateReason::ReportLoss);throw;}
    gate_.close(LocalGateReason::Kill);return *final_;
}
SimQueryResult LocalAdapterSession::work(){
    try{
        LocalSimVenue venue(config_.peer); // Constructed, queried and destroyed exclusively on this worker.
        for(;;){
            std::size_t controls=0;
            for(;controls<4;++controls){auto control=cancels_.try_pop();if(control.result!=QueueResult::Success)break;
                const auto job=*control.value;LocalAdapterOutcome outcome{{},job.order,Error::None,true,venue.cancel(job.order,job.action),job.action};
                outcome.error=outcome.reply.error;reports_.try_deliver(std::move(outcome));
            }
            auto fill=fills_.try_pop();const bool filled=fill.result==QueueResult::Success;
            if(filled){auto& job=*fill.value;LocalAdapterOutcome outcome{{},job.order,Error::None,true,venue.execute_fill(job.order,std::move(job.trade_id),job.quantity,job.price)};
                outcome.error=outcome.reply.error;reports_.try_deliver(std::move(outcome));}
            auto message=commands_.try_pop();
            if(message.result==QueueResult::Closed){if(cancels_.snapshot().size==0 && fills_.snapshot().size==0)break;continue;}
            if(message.result!=QueueResult::Success){if(controls==0 && !filled)wake_.wait();continue;}
            auto& job=*message.value;
            auto now=[&]{const auto elapsed=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now()-job.queued_at).count();
                const auto age=static_cast<std::uint64_t>(elapsed);return age>std::numeric_limits<LogicalTime>::max()-job.at?std::numeric_limits<LogicalTime>::max():job.at+age;};
            LocalAdapterOutcome outcome{job.permit,job.request.order,gate_.claim(job.permit,now()),false,{}};
            if(outcome.error==Error::None)outcome.error=gate_.try_commit_call(job.permit,now());
            if(outcome.error==Error::None){
                outcome.called=true;outcome.reply=venue.receive(job.request);outcome.error=outcome.reply.error;
                gate_.call_returned(job.permit); // Return does not attest that the order was never accepted.
            }else gate_.abort_before_call(job.permit);
            reports_.try_deliver(std::move(outcome)); // Failure synchronously closes gate before next claim.
        }
        return venue.query({config_.peer.model_epoch,1});
    }catch(...){gate_.close(LocalGateReason::ReportLoss);throw;}
}
}
