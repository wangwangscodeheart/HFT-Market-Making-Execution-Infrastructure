#pragma once
#include "p03/bounded_queue.hpp"
#include "p03/local_dispatch_gate.hpp"
namespace p03 {
// Producer failure closes the shared local gate before returning to the caller.
// Drain never reopens it. This is not a recovery certificate or a callback SDK.
template<class T> class LocalCriticalInbox {
public:
    LocalCriticalInbox(std::size_t capacity,LocalDispatchGate& gate):queue_(capacity),gate_(gate){}
    QueueResult try_deliver(T&& message){
        const auto result=queue_.try_push(std::move(message));
        if(result!=QueueResult::Success)gate_.close(LocalGateReason::ReportLoss);
        return result;
    }
    QueuePop<T> try_receive(){return queue_.try_pop();}
    void close(){gate_.close(LocalGateReason::Kill);queue_.close();}
    [[nodiscard]] QueueSnapshot snapshot() const {return queue_.snapshot();}
private:
    BoundedQueue<T> queue_;LocalDispatchGate& gate_;
};
}
