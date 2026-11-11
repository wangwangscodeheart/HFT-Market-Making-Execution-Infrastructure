#include "p03/local_view_bridge.hpp"
#include <sstream>
namespace p03 {
namespace {
std::size_t capacity(std::size_t n){if(n==0 || n>64)throw std::invalid_argument("bridge channel capacity 1..64 required");return n;}
void text(std::ostream& out,std::string_view value){constexpr char hex[]="0123456789abcdef";out<<'"';
    for(const char c:value){const auto b=static_cast<unsigned char>(c);if(c=='"' || c=='\\')out<<'\\'<<c;
        else if(b<32)out<<"\\u00"<<hex[b>>4]<<hex[b&15];else out<<c;}out<<'"';}
}
LocalViewBridge::LocalViewBridge(LocalBridgeConfig c):inputs_(capacity(c.inputs)),outputs_(capacity(c.outputs)){
    worker_=std::async(std::launch::async,[this]{work();});
}
LocalViewBridge::~LocalViewBridge(){try{close();}catch(...){failed_.store(true);}}
void LocalViewBridge::increment(std::atomic<std::uint64_t>& counter) noexcept {
    auto value=counter.load();while(value!=std::numeric_limits<std::uint64_t>::max() && !counter.compare_exchange_weak(value,value+1)){}
}
bool LocalViewBridge::valid(const LocalBridgeView& v){
    const auto bounded=[](const std::string& s){return !s.empty() && s.size()<=128;};
    if(v.snapshot==0 || v.instance==0 || !bounded(v.scope.account) || !bounded(v.scope.contract.exchange) || !bounded(v.scope.contract.instrument) ||
       !v.scope.day.valid() || v.scope.position_epoch==0 || v.order_count>v.orders.size() || v.mode<CoreMode::Ready || v.mode>CoreMode::CapacityHalted)return false;
    for(std::size_t i=0;i<v.order_count;++i){const auto& o=v.orders[i];
        if(!o.id.valid() || o.id.instance!=v.instance || o.quantity<=0 || o.quantity>max_input_lots || o.price<=0 || o.price>max_input_ticks)return false;
        for(const auto q:{o.parts.booked,o.parts.suspense,o.parts.possible,o.parts.released,o.parts.unresolved})if(q<0 || q>o.quantity)return false;
        if(o.parts.total()!=o.quantity)return false;
    }return true;
}
bool LocalViewBridge::publish(LocalBridgeView view){
    if(closed_.load() || failed_.load() || !valid(view)){increment(dropped_inputs_);return false;}
    if(inputs_.try_push(std::move(view))!=QueueResult::Success){increment(dropped_inputs_);return false;}
    increment(accepted_);wake_.notify();return true;
}
std::string LocalViewBridge::encode(const LocalBridgeView& v){
    std::ostringstream out;out<<"{\"schema\":\"p03.local-view.v1\",\"transport\":\"local_memory\",\"execution_origin\":\"LocalSimulator\",\"durable\":false,\"money_unit\":\"local_model_unit\",\"snapshot\":"<<v.snapshot<<",\"instance\":"<<v.instance<<",\"logical_at\":"<<v.at<<",\"account\":";text(out,v.scope.account);
    out<<",\"exchange\":";text(out,v.scope.contract.exchange);out<<",\"instrument\":";text(out,v.scope.contract.instrument);
    out<<",\"day\":"<<v.scope.day.ymd<<",\"position_epoch\":"<<v.scope.position_epoch<<",\"mode\":"<<static_cast<int>(v.mode)<<",\"positions\":[";
    for(std::size_t i=0;i<v.positions.size();++i){if(i)out<<',';out<<v.positions[i];}
    out<<"],\"required\":"<<v.required<<",\"fault_reasons\":"<<v.fault.reasons<<",\"loss_generation\":"<<v.fault.loss_generation
       <<",\"gate_open\":"<<(v.gate.open?"true":"false")<<",\"market_ready\":"<<(v.gate.market_ready?"true":"false")<<",\"heartbeat_enabled\":"<<(v.gate.heartbeat_enabled?"true":"false")
       <<",\"calls_quiesced\":"<<(v.gate.quiesced?"true":"false")<<",\"orders\":[";
    for(std::size_t i=0;i<v.order_count;++i){if(i)out<<',';const auto& o=v.orders[i];
        out<<"{\"instance\":"<<o.id.instance<<",\"serial\":"<<o.id.serial<<",\"side\":"<<static_cast<int>(o.side)<<",\"offset\":"<<static_cast<int>(o.offset)
           <<",\"price\":"<<o.price<<",\"quantity\":"<<o.quantity<<",\"execution\":"<<static_cast<int>(o.execution)<<",\"parts\":["
           <<o.parts.booked<<','<<o.parts.suspense<<','<<o.parts.possible<<','<<o.parts.released<<','<<o.parts.unresolved<<"]}";
    }out<<"]}";auto result=out.str();if(result.size()>65536)throw std::length_error("bounded bridge JSON exceeded");return result;
}
void LocalViewBridge::work(){
    try{for(;;){auto view=inputs_.try_pop();if(view.result==QueueResult::Closed)break;if(view.result!=QueueResult::Success){wake_.wait();continue;}
        try{auto json=encode(*view.value);increment(encoded_);if(outputs_.try_push(std::move(json))!=QueueResult::Success)increment(dropped_outputs_);}
        catch(...){increment(encoding_errors_);increment(dropped_outputs_);}
    }}catch(...){failed_.store(true);}
}
void LocalViewBridge::close(){std::lock_guard lock(join_mutex_);closed_.store(true);inputs_.close();wake_.close();if(worker_.valid())worker_.get();outputs_.close();}
LocalBridgeStats LocalViewBridge::stats() const {return {accepted_.load(),encoded_.load(),dropped_inputs_.load(),dropped_outputs_.load(),encoding_errors_.load(),closed_.load(),failed_.load()};}
}
