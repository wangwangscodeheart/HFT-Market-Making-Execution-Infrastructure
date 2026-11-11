#pragma once
#include "p03/local_sim.hpp"
#include <deque>
namespace p03 {
// Bounded in-memory receipt queue. Failed delivery remains at front until retried.
class LocalEvidenceInbox {
public:
    explicit LocalEvidenceInbox(std::size_t capacity):capacity_(capacity){
        if(capacity==0 || capacity>8192)throw std::invalid_argument("bounded evidence inbox required");
    }
    Error enqueue(const std::vector<BusinessEvent>& reports){
        if(reports.size()>capacity_-pending_.size())return Error::Capacity;
        auto candidate=pending_;for(const auto& report:reports)candidate.push_back(report);
        pending_.swap(candidate);return Error::None;
    }
    [[nodiscard]] std::size_t size() const noexcept {return pending_.size();}
    [[nodiscard]] bool contains_exact(const TradeReport& report) const {
        for(const auto& event:pending_)if(const auto* queued=std::get_if<TradeReport>(&event))
            if(queued->trade==report.trade && queued->stamp==report.stamp && queued->channel==report.channel)return true;
        return false;
    }
    [[nodiscard]] const BusinessEvent& front() const {return pending_.at(0);}
    void acknowledge(){if(pending_.empty())throw std::logic_error("empty inbox acknowledgement");pending_.pop_front();}
private:
    std::size_t capacity_;std::deque<BusinessEvent> pending_;
};
} // namespace p03
