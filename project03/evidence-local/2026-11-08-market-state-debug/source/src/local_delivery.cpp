#include "p03/local_delivery.hpp"

namespace p03 {
LocalReportDelivery::LocalReportDelivery(std::size_t reports,std::size_t attempts)
    :report_capacity_(reports),attempt_capacity_(attempts){
    if(reports==0 || reports>4096 || attempts==0 || attempts>8192)
        throw std::invalid_argument("bounded local delivery capacities required");
    reports_.reserve(reports);attempts_.reserve(attempts);
}
Error LocalReportDelivery::capture(const SimReply& reply){
    if(reply.reports.size()>report_capacity_-reports_.size())return Error::Capacity;
    for(const auto& event:reply.reports){
        const auto valid=std::visit([](const auto& report){
            using T=std::decay_t<decltype(report)>;
            if constexpr(std::is_same_v<T,TradeReport>)return report.stamp.origin==EvidenceOrigin::LocalSimulator;
            else if constexpr(std::is_same_v<T,WorkingReport> || std::is_same_v<T,TerminalReport> || std::is_same_v<T,CancelReport>)
                return report.header.stamp.origin==EvidenceOrigin::LocalSimulator;
            else return false;
        },event);
        if(!valid)return Error::UntrustedEvidence;
    }
    // Copy the complete batch before publishing any capture IDs.
    auto candidate=reports_;
    for(const auto& event:reply.reports)candidate.push_back({static_cast<std::uint64_t>(candidate.size()+1),reply.revision,event});
    reports_.swap(candidate);return Error::None;
}
StepResult LocalReportDelivery::deliver(CoreLoop& core,std::uint64_t id,LogicalTime at){
    if(id==0 || id>reports_.size())return {Error::InvalidInput};
    if(attempts_.size()>=attempt_capacity_)return {Error::Capacity};
    const auto result=core.step(reports_[static_cast<std::size_t>(id-1)].event,at);
    attempts_.push_back({id,at,result});return result;
}
} // namespace p03
