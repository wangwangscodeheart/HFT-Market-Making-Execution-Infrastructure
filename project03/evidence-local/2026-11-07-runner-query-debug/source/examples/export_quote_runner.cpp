#include "quote_runner_scenario.hpp"
#include "json_output.hpp"
#include <iostream>
namespace {
using namespace p03;using namespace p03::example;
void id(OrderId value){std::cout<<'"'<<value.instance<<':'<<value.serial<<'"';}
void intent(const OrderIntent& i){
    std::cout<<"{\"account\":";json_text(std::cout,i.account);std::cout<<",\"instrument\":";json_text(std::cout,i.instrument);
    std::cout<<",\"day\":"<<i.day.ymd<<",\"side\":"<<static_cast<int>(i.side)<<",\"offset\":"<<static_cast<int>(i.offset)
        <<",\"price\":"<<i.price<<",\"quantity\":"<<i.quantity<<'}';
}
void input(const LocalQuoteInput& value){
    if(const auto* pulse=std::get_if<MarketPulse>(&value)){
        const auto& b=pulse->book;std::cout<<"{\"type\":\"MarketPulse\",\"source\":";json_text(std::cout,b.source);
        std::cout<<",\"epoch\":"<<b.epoch<<",\"sequence\":"<<b.sequence<<",\"received_at\":"<<b.received_at
            <<",\"bid\":"<<b.bid<<",\"ask\":"<<b.ask<<",\"bid_quantity\":"<<b.bid_quantity<<",\"ask_quantity\":"<<b.ask_quantity
            <<",\"healthy\":"<<(b.healthy?"true":"false")<<",\"session_second\":"<<pulse->session.second<<'}';
    }else if(const auto* fill=std::get_if<LocalFillInput>(&value)){
        std::cout<<"{\"type\":\"LocalFillInput\",\"order\":";id(fill->order);std::cout<<",\"trade_id\":";json_text(std::cout,fill->trade_id);
        std::cout<<",\"quantity\":"<<fill->quantity<<",\"price\":"<<fill->price<<'}';
    }else if(std::holds_alternative<LocalQuoteTimer>(value))std::cout<<"{\"type\":\"LocalQuoteTimer\"}";
    else if(const auto* held=std::get_if<LocalHeldReportInput>(&value))std::cout<<"{\"type\":\"LocalHeldReportInput\",\"index\":"<<held->index<<'}';
    else if(const auto* query=std::get_if<LocalTradeQueryInput>(&value))std::cout<<"{\"type\":\"LocalTradeQueryInput\",\"query_instance\":"<<query->query.instance<<",\"query_serial\":"<<query->query.serial<<",\"omit_last_trade\":"<<(query->omit_last_trade?"true":"false")<<'}';
    else if(const auto* deferred=std::get_if<LocalDeferredFillInput>(&value)){
        std::cout<<"{\"type\":\"LocalDeferredFillInput\",\"order\":";id(deferred->fill.order);std::cout<<",\"trade_id\":";json_text(std::cout,deferred->fill.trade_id);
        std::cout<<",\"quantity\":"<<deferred->fill.quantity<<",\"price\":"<<deferred->fill.price<<'}';
    }
    else std::cout<<"{\"type\":\"StopLocalQuotes\"}";
}
void event(const BusinessEvent& value){
    std::visit([](const auto& e){using T=std::decay_t<decltype(e)>;
        if constexpr(std::is_same_v<T,ReferenceMark>)std::cout<<"{\"type\":\"ReferenceMark\",\"price\":"<<e.price<<'}';
        else if constexpr(std::is_same_v<T,SubmitOrder>){std::cout<<"{\"type\":\"SubmitOrder\",\"intent\":";intent(e.intent);std::cout<<'}';}
        else if constexpr(std::is_same_v<T,RequestCancel>){std::cout<<"{\"type\":\"RequestCancel\",\"order\":";id(e.order);std::cout<<'}';}
        else if constexpr(std::is_same_v<T,TradeReport>){std::cout<<"{\"type\":\"TradeReport\",\"order\":";id(e.trade.order);std::cout<<",\"quantity\":"<<e.trade.quantity<<",\"price\":"<<e.trade.price<<'}';}
        else if constexpr(std::is_same_v<T,WorkingReport> || std::is_same_v<T,CancelReport> || std::is_same_v<T,TerminalReport>){
            std::cout<<"{\"type\":\""<<(std::is_same_v<T,WorkingReport>?"WorkingReport":std::is_same_v<T,CancelReport>?"CancelReport":"TerminalReport")<<"\",\"order\":";id(e.header.order);std::cout<<'}';
        }else if constexpr(std::is_same_v<T,KillRequested>)std::cout<<"{\"type\":\"KillRequested\"}";
        else std::cout<<"{\"type\":\"Other\"}";
    },value);
}
}
int main(int argc,char** argv){
    if(argc>2 || (argc==2 && std::string_view(argv[1])!="--expiry"))return 2;
    const bool expiry=argc==2;
    const auto c=quote_runner_config();LocalQuoteRunner run(c),replay(c);bool passed=expiry?run_quote_expiry_scenario(run,c):run_quote_scenario(run,c);
    for(const auto& f:run.trace())passed=(replay.step(f.input,f.at)==f.error)&&passed;
    passed=passed && run.core().canonical_state()==replay.core().canonical_state();
    std::cout<<"{\"schema\":\"p03.quote-runner.v1\",\"scenario\":\""<<(expiry?"market-expiry":"quote-cycle")<<"\",\"max_book_age\":"<<c.strategy.max_age<<",\"market_origin\":\"InterfaceFixture\",\"execution_origin\":\"LocalSimulator\",\"historical_trace\":false,\"time_unit\":\"logical_step\",\"passed\":"<<(passed?"true":"false")<<",\"frames\":[";
    bool first=true;for(const auto& f:run.trace()){
        if(!first)std::cout<<',';first=false;
        std::cout<<"{\"ordinal\":"<<f.ordinal<<",\"at\":"<<f.at<<",\"input\":";input(f.input);
        std::cout<<",\"error\":";json_text(std::cout,error_name(f.error));std::cout<<",\"completed\":"<<(f.completed?"true":"false")
            <<",\"mode\":"<<static_cast<int>(f.mode)<<",\"core_positions\":";json_numbers(std::cout,f.positions);
        std::cout<<",\"peer_positions\":";json_numbers(std::cout,f.peer->positions);std::cout<<",\"peer_revision\":"<<f.peer->revision;
        std::cout<<",\"pending_reports\":"<<f.pending_reports;
        std::cout<<",\"decision\":";
        if(!f.decision)std::cout<<"null";
        else {std::cout<<"{\"error\":";json_text(std::cout,error_name(f.decision->error));
            std::cout<<",\"withdraw\":"<<(f.decision->withdraw?"true":"false")<<",\"generation\":"<<(f.decision->target?f.decision->target->generation:0)<<'}';}
        std::cout<<",\"orders\":[";bool order_first=true;for(const auto& o:f.orders){if(!order_first)std::cout<<',';order_first=false;
            std::cout<<"{\"id\":";id(o.id);std::cout<<",\"intent\":";intent(o.intent);const auto p=o.parts;
            std::cout<<",\"parts\":";json_numbers(std::cout,std::array{p.booked,p.suspense,p.possible,p.released,p.unresolved});
            std::cout<<",\"execution\":"<<static_cast<int>(o.execution)<<'}';}
        std::cout<<"],\"calls\":[";bool call_first=true;for(const auto& call:f.calls){if(!call_first)std::cout<<',';call_first=false;
            std::cout<<"{\"input\":";event(call.input);std::cout<<",\"core_sequence\":"<<call.result.core_sequence<<",\"ingress\":"<<call.result.ingress_sequence<<",\"error\":";
            json_text(std::cout,error_name(call.result.error));std::cout<<'}';}
        std::cout<<"],\"canonical\":";json_text(std::cout,f.canonical);std::cout<<'}';
    }std::cout<<"]}\n";return passed?0:1;
}
