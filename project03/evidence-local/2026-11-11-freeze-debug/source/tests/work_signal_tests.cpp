#include "test.hpp"
#include "p03/work_signal.hpp"
#include <future>
#include <thread>
using namespace p03;
namespace {
bool waiting(WorkSignal& signal){const auto until=std::chrono::steady_clock::now()+std::chrono::seconds(3);
    while(std::chrono::steady_clock::now()<until){if(signal.snapshot().waiting)return true;std::this_thread::yield();}return false;}
}
TEST_CASE(WAKE_notification_before_wait_is_retained_and_notifications_coalesce){
    WorkSignal signal;CHECK(signal.notify());CHECK(signal.notify());CHECK(signal.snapshot().pending);
    CHECK(signal.wait());CHECK(!signal.snapshot().pending);signal.close();CHECK(!signal.wait());CHECK(!signal.notify());
}
TEST_CASE(WAKE_real_thread_wait_is_released_by_notification_after_empty_check){
    WorkSignal signal;auto consumer=std::async(std::launch::async,[&]{return signal.wait();});
    const bool observed=waiting(signal);const bool notified=signal.notify();const bool result=consumer.get();signal.close();
    CHECK(observed);CHECK(notified);CHECK(result);CHECK(!signal.snapshot().waiting);
}
TEST_CASE(WAKE_close_wakes_idle_consumer_and_cannot_be_reopened){
    WorkSignal signal;auto consumer=std::async(std::launch::async,[&]{return signal.wait();});
    const bool observed=waiting(signal);signal.close();signal.close();const bool result=consumer.get();
    CHECK(observed);CHECK(!result);CHECK(!signal.notify());CHECK(!signal.wait());CHECK(signal.snapshot().closed);
}
