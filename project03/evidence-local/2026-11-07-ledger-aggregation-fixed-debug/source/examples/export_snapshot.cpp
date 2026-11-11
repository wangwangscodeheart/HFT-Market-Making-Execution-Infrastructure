#include "demo_scenario.hpp"
#include <iostream>
#include <string_view>

namespace {
void text(std::string_view value) {
    std::cout << '"';
    constexpr char hex[]="0123456789abcdef";
    for(const char c:value) {
        const auto byte=static_cast<unsigned char>(c);
        if(c=='"' || c=='\\') std::cout << '\\' << c;
        else if(byte<32) std::cout << "\\u00" << hex[byte>>4] << hex[byte&15];
        else std::cout << c;
    }
    std::cout << '"';
}
template<class Range> void numbers(const Range& values) {
    std::cout << '['; bool first=true;
    for(const auto value:values){if(!first)std::cout<<',';first=false;std::cout<<value;}
    std::cout << ']';
}
}
int main(int argc,char** argv) {
    using namespace p03;
    const auto scenario=example::select_scenario(argc,argv);
    StepHarness harness(scenario.config);
    constexpr std::string_view execution[]{"Created","Working","PartiallyFilled","Filled","Canceled","Rejected","NotSent"};
    constexpr std::string_view cancellation[]{"None","Requested","Succeeded","Rejected","Unknown"};
    constexpr std::string_view consistency[]{"InSync","MissingTrades","Unreconciled","Conflict"};
    std::cout << "{\"schema\":\"p03.offline.v1\",\"origin\":\"LocalFixture\",\"producer\":\"p03_export_snapshot\",\"account\":";
    text(scenario.config.anchor.account); std::cout<<",\"instrument\":";text(scenario.config.contract.instrument);
    std::cout<<",\"strategy\":";text(harness.core().config().strategy.value);
    std::cout<<",\"business_scope\":{\"account\":";text(harness.core().scope().account);
    std::cout<<",\"exchange\":";text(harness.core().scope().contract.exchange);
    std::cout<<",\"instrument\":";text(harness.core().scope().contract.instrument);
    std::cout<<",\"trading_day\":"<<harness.core().scope().day.ymd;
    std::cout<<",\"position_epoch\":"<<harness.core().scope().position_epoch<<'}';
    std::cout<<",\"scenario\":";text(scenario.key);
    std::cout<<",\"source_url\":";text(scenario.source_url);
    std::cout<<",\"source_ref\":";text(scenario.source_ref);
    std::cout<<",\"source_order_quantity\":"<<(scenario.key=="normal"?10000:0);
    std::cout<<",\"quantity_scale\":"<<(scenario.key=="normal"?1000:1);
    std::cout << ",\"frames\":[";
    LogicalTime time=0;
    for(const auto& input:scenario.inputs) {
        if(harness.schedule(time,input)!=Error::None || harness.run()!=Error::None) return 1;
        const auto& s=harness.core().state(); const auto& result=harness.trace().back().result;
        if(time!=0)std::cout<<',';
        std::cout<<"{\"sequence\":"<<result.core_sequence<<",\"logical_time\":"<<time<<",\"event\":";
        const auto name=std::visit([](const auto& event)->std::string_view{
            using T=std::decay_t<decltype(event)>;
            if constexpr(std::is_same_v<T,ReferenceMark>)return "ReferenceMark";
            else if constexpr(std::is_same_v<T,SubmitOrder>)return "SubmitOrder";
            else if constexpr(std::is_same_v<T,WorkingReport>)return "WorkingReport";
            else if constexpr(std::is_same_v<T,TradeReport>)return "TradeReport";
            else if constexpr(std::is_same_v<T,RequestCancel>)return "RequestCancel";
            else if constexpr(std::is_same_v<T,TerminalReport>)return "TerminalReport";
            else if constexpr(std::is_same_v<T,RequestFutureOperation>)return "RequestFutureOperation";
            else return "OtherBusinessEvent";
        },input);
        text(name);
        std::cout<<",\"ingress_sequence\":"<<result.ingress_sequence<<",\"admission\":";text(admission_name(result.admission));
        std::cout<<",\"error\":";text(error_name(result.error));
        std::cout<<",\"applied\":"<<(result.applied?"true":"false")<<",\"invariant\":"<<(harness.core().invariant()?"true":"false");
        std::cout<<",\"positions\":";numbers(s.positions.booked());
        std::cout<<",\"frozen\":";numbers(s.positions.frozen());
        std::cout<<",\"position_anchor\":";numbers(s.positions.anchor().booked);
        const auto found=s.orders.find(scenario.order);
        std::cout<<",\"order\":";
        if(found==s.orders.end())std::cout<<"null";
        else {
            const auto& order=found->second; const auto p=order.parts();
            std::cout<<"{\"id\":\""<<order.id().instance<<':'<<order.id().serial<<"\",\"quantity\":"<<order.intent().quantity<<",\"price\":"<<order.intent().price<<",\"parts\":";
            numbers(std::array{p.booked,p.suspense,p.possible,p.released,p.unresolved});
            std::cout<<",\"execution\":";text(execution[static_cast<std::size_t>(order.execution())]);
            std::cout<<",\"cancel\":";text(cancellation[static_cast<std::size_t>(order.cancel())]);
            std::cout<<",\"consistency\":";text(consistency[static_cast<std::size_t>(order.consistency())]);
            std::cout<<",\"strategy\":";text(order.strategy().value);
            std::cout<<",\"client_id\":";
            if(order.client_id()){
                std::cout<<"{\"producer\":";text(order.client_id()->producer);
                std::cout<<",\"value\":";text(order.client_id()->value);std::cout<<'}';
            }else std::cout<<"null";
            std::cout<<",\"send\":\"NotImplemented\"}";
        }
        std::cout<<",\"trade_id\":";
        const auto* trade=std::get_if<TradeReport>(&input);text(trade?trade->trade.key.trade_id:"");
        std::cout<<",\"last_quantity\":"<<(trade?trade->trade.quantity:0);
        std::cout<<",\"trade_records\":"<<s.executions.records().size()<<",\"funds_required\":"<<s.funds.view().local_required;
        std::cout<<",\"canonical_state\":";text(harness.core().canonical_state());std::cout<<'}';
        ++time;
    }
    std::cout<<"]}\n";
    return harness.core().invariant() && harness.core().state().orders.at(scenario.order).parts()==scenario.expected ? 0 : 1;
}
