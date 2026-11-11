#include "test.hpp"
#include "p03/bounded_queue.hpp"
#include <future>
#include <array>
#include <memory>
#include <thread>
using namespace p03;
namespace {
using Clock=std::chrono::steady_clock;
auto deadline(){return Clock::now()+std::chrono::seconds(3);}
template<class Predicate> bool observe(Predicate predicate){
    const auto until=deadline();while(Clock::now()<until){if(predicate())return true;std::this_thread::yield();}return false;
}
}
TEST_CASE(QUEUE_capacity_fifo_wraparound_and_failed_push_preserves_move_only_message){
    CHECK_THROWS(BoundedQueue<int>(0),std::invalid_argument);CHECK_THROWS(BoundedQueue<int>(65537),std::invalid_argument);
    BoundedQueue<std::unique_ptr<int>> q(2);CHECK_EQ(q.try_pop().result,QueueResult::Empty);
    CHECK_EQ(q.try_push(std::make_unique<int>(1)),QueueResult::Success);CHECK_EQ(q.try_push(std::make_unique<int>(2)),QueueResult::Success);
    auto rejected=std::make_unique<int>(3);CHECK_EQ(q.try_push(std::move(rejected)),QueueResult::Full);CHECK(rejected && *rejected==3);
    auto first=q.try_pop();CHECK_EQ(first.result,QueueResult::Success);CHECK_EQ(**first.value,1);
    CHECK_EQ(q.try_push(std::move(rejected)),QueueResult::Success);CHECK(!rejected);
    CHECK_EQ(**q.try_pop().value,2);CHECK_EQ(**q.try_pop().value,3);CHECK_EQ(q.snapshot().high_water,2U);
}
TEST_CASE(QUEUE_deadlines_do_not_consume_messages_and_close_drains_before_closed){
    BoundedQueue<std::unique_ptr<int>> q(1);CHECK_EQ(q.pop_until(Clock::now()).result,QueueResult::Timeout);
    CHECK_EQ(q.try_push(std::make_unique<int>(1)),QueueResult::Success);auto value=std::make_unique<int>(2);
    CHECK_EQ(q.push_until(std::move(value),Clock::now()),QueueResult::Timeout);CHECK(value && *value==2);
    q.close();q.close();CHECK_EQ(q.try_push(std::move(value)),QueueResult::Closed);CHECK(value);
    CHECK_EQ(**q.pop_until(Clock::now()).value,1);CHECK_EQ(q.pop_until(Clock::now()).result,QueueResult::Closed);
    CHECK_EQ(q.snapshot().size,0U);CHECK(q.snapshot().closed);
}
TEST_CASE(QUEUE_close_wakes_all_blocked_consumers){
    BoundedQueue<int> q(1);auto receive=[&]{return q.pop_until(deadline()).result;};
    auto a=std::async(std::launch::async,receive),b=std::async(std::launch::async,receive);
    const bool waiting=observe([&]{return q.snapshot().waiting_pop==2;});q.close();
    const auto ar=a.get(),br=b.get();CHECK(waiting);CHECK_EQ(ar,QueueResult::Closed);CHECK_EQ(br,QueueResult::Closed);
    CHECK_EQ(q.snapshot().waiting_pop,0U);
}
TEST_CASE(QUEUE_close_wakes_all_blocked_producers_without_moving_rejected_values){
    BoundedQueue<std::unique_ptr<int>> q(1);CHECK_EQ(q.try_push(std::make_unique<int>(1)),QueueResult::Success);
    auto send=[&]{auto v=std::make_unique<int>(2);auto r=q.push_until(std::move(v),deadline());return std::pair{r,v!=nullptr};};
    auto a=std::async(std::launch::async,send),b=std::async(std::launch::async,send);
    const bool waiting=observe([&]{return q.snapshot().waiting_push==2;});q.close();
    const auto ar=a.get(),br=b.get();CHECK(waiting);CHECK_EQ(ar.first,QueueResult::Closed);CHECK(ar.second);CHECK_EQ(br.first,QueueResult::Closed);CHECK(br.second);
    CHECK_EQ(q.snapshot().size,1U);CHECK_EQ(q.snapshot().waiting_push,0U);
}
TEST_CASE(QUEUE_pop_releases_blocked_producer_and_push_releases_blocked_consumer){
    BoundedQueue<int> q(1);CHECK_EQ(q.try_push(1),QueueResult::Success);
    auto producer=std::async(std::launch::async,[&]{return q.push_until(2,deadline());});
    const bool push_waiting=observe([&]{return q.snapshot().waiting_push==1;});auto first=q.try_pop();const auto sent=producer.get();
    CHECK(push_waiting);CHECK_EQ(*first.value,1);CHECK_EQ(sent,QueueResult::Success);CHECK_EQ(*q.try_pop().value,2);
    auto consumer=std::async(std::launch::async,[&]{return q.pop_until(deadline());});
    const bool pop_waiting=observe([&]{return q.snapshot().waiting_pop==1;});const auto pushed=q.try_push(3);auto received=consumer.get();q.close();
    CHECK(pop_waiting);CHECK_EQ(pushed,QueueResult::Success);CHECK_EQ(received.result,QueueResult::Success);CHECK_EQ(*received.value,3);
}
TEST_CASE(QUEUE_real_threads_transfer_fifo_and_drain_after_producer_close){
    BoundedQueue<int> q(7);
    auto producer=std::async(std::launch::async,[&]{for(int i=0;i<2000;++i)if(q.push_until(std::move(i),deadline())!=QueueResult::Success){q.close();return false;}q.close();return true;});
    auto consumer=std::async(std::launch::async,[&]{int expected=0;for(;;){auto r=q.pop_until(deadline());if(r.result==QueueResult::Closed)return expected==2000;if(r.result!=QueueResult::Success || !r.value || *r.value!=expected++){q.close();return false;}}});
    const bool sent=producer.get(),received=consumer.get();CHECK(sent);CHECK(received);CHECK_EQ(q.snapshot().size,0U);CHECK(q.snapshot().high_water<=7);
}
TEST_CASE(QUEUE_multiple_producers_preserve_each_producer_order_without_loss){
    BoundedQueue<int> q(3);
    auto send=[&](int id){for(int i=0;i<1000;++i){int v=id*1000+i;if(q.push_until(std::move(v),deadline())!=QueueResult::Success){q.close();return false;}}return true;};
    auto consumer=std::async(std::launch::async,[&]{std::array<int,2> next{};int count=0;for(;;){
        auto r=q.pop_until(deadline());if(r.result==QueueResult::Closed)return count==2000 && next==std::array<int,2>{1000,1000};
        if(r.result!=QueueResult::Success || !r.value || *r.value<0 || *r.value>=2000){q.close();return false;}
        const auto id=static_cast<std::size_t>(*r.value/1000);if(*r.value%1000!=next[id]++){q.close();return false;}++count;
    }});
    auto a=std::async(std::launch::async,send,0),b=std::async(std::launch::async,send,1);
    const bool sent_a=a.get(),sent_b=b.get();q.close();const bool received=consumer.get();
    CHECK(sent_a);CHECK(sent_b);CHECK(received);CHECK_EQ(q.snapshot().size,0U);CHECK(q.snapshot().high_water<=3);
}
