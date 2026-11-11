#pragma once
#include <condition_variable>
#include <mutex>
namespace p03 {
struct WorkSignalSnapshot {bool pending{},closed{},waiting{};};
// Single-consumer coalescing wake signal. Queues remain the authority for work.
// A notification before wait is retained; close wakes the consumer permanently.
class WorkSignal {
public:
    bool notify(){
        {std::lock_guard lock(mutex_);if(closed_)return false;pending_=true;}
        cv_.notify_one();return true;
    }
    bool wait(){
        std::unique_lock lock(mutex_);waiting_=true;
        try{cv_.wait(lock,[&]{return pending_ || closed_;});}catch(...){waiting_=false;throw;}
        waiting_=false;pending_=false;return !closed_;
    }
    void close(){
        {std::lock_guard lock(mutex_);closed_=true;}
        cv_.notify_all();
    }
    [[nodiscard]] WorkSignalSnapshot snapshot() const {
        std::lock_guard lock(mutex_);return {pending_,closed_,waiting_};
    }
private:
    mutable std::mutex mutex_;std::condition_variable cv_;bool pending_{},closed_{},waiting_{};
};
}
