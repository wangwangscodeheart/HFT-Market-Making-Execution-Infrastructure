#include "p03/local_runtime_runner.hpp"
namespace p03 {
LocalRuntimeRunner::LocalRuntimeRunner(LocalRuntimeConfig c)
    :owner_(std::this_thread::get_id()),core_(std::move(c.core)),session_(core_,std::move(c.adapter)),normal_capacity_(c.inputs),cleanup_capacity_(c.cleanup_inputs){
    if(normal_capacity_==0 || normal_capacity_>1024 || cleanup_capacity_==0 || cleanup_capacity_>128 ||
       core_.config().capacity.orders>64 || core_.config().capacity.trades>256)throw std::invalid_argument("bounded local runtime profile required");
    trace_.reserve(normal_capacity_+cleanup_capacity_);
}
void LocalRuntimeRunner::require_owner() const {if(std::this_thread::get_id()!=owner_)throw std::logic_error("runtime owner required");}
Error LocalRuntimeRunner::step(LocalRuntimeInput input,LogicalTime at){
    require_owner();if(fatal_)return Error::Halted;if(!trace_.empty() && at<last_time_)return Error::OutOfOrderTime;
    const bool cleanup=std::holds_alternative<KillRequested>(input) || std::holds_alternative<RuntimeCancel>(input) ||
        std::holds_alternative<RuntimePump>(input) || std::holds_alternative<RuntimeRetry>(input) || std::holds_alternative<RuntimeFinish>(input);
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
        std::visit([&](const auto& value){using T=std::decay_t<decltype(value)>;
            if constexpr(std::is_same_v<T,RuntimeStart>){if(started_)f.error=Error::InvalidInput;else{session_.start();started_=true;}}
            else if constexpr(std::is_same_v<T,ReferenceMark>){f.core_result=core_.step(value,at);f.error=f.core_result->error;}
            else if constexpr(std::is_same_v<T,SubmitOrder>){const auto result=session_.submit(value,at);f.core_result=result.core;f.error=result.dispatch;}
            else if constexpr(std::is_same_v<T,RuntimeCancel>){const auto result=session_.cancel(value.order,at);f.core_result=result.core;f.error=result.dispatch;}
            else if constexpr(std::is_same_v<T,RuntimeFill>)f.error=session_.simulate_fill(value.order,value.trade_id,value.quantity,value.price);
            else if constexpr(std::is_same_v<T,KillRequested>){f.core_result=session_.kill(at);f.error=f.core_result->error;}
            else if constexpr(std::is_same_v<T,RuntimePump>){f.pump_result=session_.pump(at,value.budget);f.error=f.pump_result->error;}
            else if constexpr(std::is_same_v<T,RuntimeRetry>)f.error=session_.retry_reports(at);
            else if constexpr(std::is_same_v<T,RuntimeFinish>){peer_query_=session_.finish();finished_=true;started_=true;f.error=peer_query_->error;}
        },f.input);
        f.gate=session_.gate_snapshot();f.canonical=core_.canonical_state();f.completed=true;
    }catch(...){fatal_=true;stopped_=true;session_.gate_.close(LocalGateReason::Capacity);f.error=Error::InjectedFailure;throw;}
    return f.error;
}
}
