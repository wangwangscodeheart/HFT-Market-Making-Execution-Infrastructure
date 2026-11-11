#pragma once
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>
namespace p03 {
enum class QueueResult {Success,Full,Empty,Closed,Timeout};
struct QueueSnapshot {std::size_t capacity{},size{},high_water{},waiting_push{},waiting_pop{};bool closed{};};
template<class T> struct QueuePop {QueueResult result;std::optional<T> value;};
// Fixed slot count, FIFO, mutex/CV. Message-owned heap memory must be bounded
// by the channel's profile separately. Owner must close and join before destruction.
template<class T> class BoundedQueue {
    static_assert(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_destructible_v<T>);
public:
    using Clock=std::chrono::steady_clock;
    explicit BoundedQueue(std::size_t capacity):slots_(checked_capacity(capacity)){}
    BoundedQueue(const BoundedQueue&)=delete;
    BoundedQueue& operator=(const BoundedQueue&)=delete;
    QueueResult try_push(T&& value){
        std::unique_lock lock(mutex_);
        if(closed_)return QueueResult::Closed;
        if(size_==slots_.size())return QueueResult::Full;
        return push_locked(std::move(value));
    }
    QueueResult push_until(T&& value,Clock::time_point deadline){
        std::unique_lock lock(mutex_);
        ++waiting_push_;
        const bool ready=space_.wait_until(lock,deadline,[&]{return closed_ || size_<slots_.size();});
        --waiting_push_;
        if(closed_)return QueueResult::Closed;
        if(!ready)return QueueResult::Timeout;
        return push_locked(std::move(value));
    }
    QueuePop<T> try_pop(){
        std::unique_lock lock(mutex_);
        if(size_==0)return {closed_?QueueResult::Closed:QueueResult::Empty,{}};
        return pop_locked();
    }
    QueuePop<T> pop_until(Clock::time_point deadline){
        std::unique_lock lock(mutex_);
        ++waiting_pop_;
        const bool ready=data_.wait_until(lock,deadline,[&]{return closed_ || size_!=0;});
        --waiting_pop_;
        if(size_!=0)return pop_locked(); // Closing rejects producers but drains accepted messages.
        return {closed_?QueueResult::Closed:ready?QueueResult::Empty:QueueResult::Timeout,{}};
    }
    void close(){
        {std::lock_guard lock(mutex_);closed_=true;}
        space_.notify_all();data_.notify_all();
    }
    [[nodiscard]] QueueSnapshot snapshot() const {
        std::lock_guard lock(mutex_);
        return {slots_.size(),size_,high_water_,waiting_push_,waiting_pop_,closed_};
    }
private:
    static std::size_t checked_capacity(std::size_t n){if(n==0 || n>65536)throw std::invalid_argument("bounded queue capacity 1..65536 required");return n;}
    QueueResult push_locked(T&& value){
        slots_[(head_+size_)%slots_.size()].emplace(std::move(value));
        ++size_;if(size_>high_water_)high_water_=size_;data_.notify_one();return QueueResult::Success;
    }
    QueuePop<T> pop_locked(){
        QueuePop<T> result{QueueResult::Success,std::optional<T>{std::move(*slots_[head_])}};
        slots_[head_].reset();head_=(head_+1)%slots_.size();--size_;space_.notify_one();return result;
    }
    std::vector<std::optional<T>> slots_;
    mutable std::mutex mutex_;std::condition_variable data_,space_;
    std::size_t head_{},size_{},high_water_{},waiting_push_{},waiting_pop_{};bool closed_{};
};
} // namespace p03
