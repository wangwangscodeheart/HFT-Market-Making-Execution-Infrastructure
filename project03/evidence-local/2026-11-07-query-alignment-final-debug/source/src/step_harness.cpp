#include "p03/step_harness.hpp"
#include <algorithm>

namespace p03 {
Error StepHarness::schedule(LogicalTime at,BusinessEvent e) {
    if(at<time_) return Error::OutOfOrderTime;
    if(pending_.size()>=capacity_ || trace_.size()+pending_.size()>=capacity_) return Error::Capacity;
    if(insertion_==std::numeric_limits<std::uint64_t>::max()) return Error::IdExhausted;
    pending_.push_back({at,++insertion_,std::move(e)}); return Error::None;
}
Error StepHarness::run() {
    std::sort(pending_.begin(),pending_.end(),[](const auto& a,const auto& b){
        return std::pair{a.at,a.insertion}<std::pair{b.at,b.insertion};
    });
    for(const auto& e:pending_) {
        time_=e.at; auto r=core_.step(e.event,e.at);
        if(!core_.invariant()) throw std::logic_error("StepHarness invariant failure");
        trace_.push_back({e.at,e.event,r,core_.canonical_state()});
    }
    pending_.clear(); return Error::None;
}
} // namespace p03
