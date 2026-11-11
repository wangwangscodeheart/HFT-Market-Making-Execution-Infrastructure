#include "fixture.hpp"
#include "p03/order_planner.hpp"
using namespace fixture;
TEST_CASE(PLANNER_unchanged_open_quote_is_replaced_when_confirmed_opposite_inventory_appears){
    Fixture f;const auto buy=f.submit(intent(Side::Buy,Offset::Open,1));
    const auto sell=f.submit(intent(Side::Sell,Offset::Open,1,501));CHECK_EQ(f.apply(trade(buy,"routing-fill",1)).error,Error::None);
    QuoteTarget t{f.core.scope(),f.core.config().strategy,1,session(),{},QuoteLevel{501,1}};
    const auto plan=plan_quotes(f.core,t,1);CHECK_EQ(plan.error,Error::None);CHECK_EQ(plan.intents.size(),1U);
    CHECK_EQ(std::get<RequestCancel>(plan.intents[0]).order,sell);
}
TEST_CASE(PLANNER_existing_close_uses_its_own_frozen_quantity_without_cancel_churn){
    Fixture f(config({1,0,0,0}));f.submit(intent(Side::Sell,Offset::CloseToday,1,501));
    QuoteTarget t{f.core.scope(),f.core.config().strategy,1,session(),{},QuoteLevel{501,3}};
    CHECK_EQ(f.core.state().positions.frozen()[0],1);
    const auto plan=plan_quotes(f.core,t,1);CHECK_EQ(plan.error,Error::None);CHECK(plan.intents.empty());
}
namespace {
QuoteTarget target(const CoreLoop& core){return {core.scope(),core.config().strategy,1,session(),QuoteLevel{499,1},QuoteLevel{501,1}};}
}
TEST_CASE(PLANNER_two_sided_plan_is_read_only_and_each_submit_still_passes_core_risk){
    Fixture f;const auto before=f.core.canonical_state();const auto plan=plan_quotes(f.core,target(f.core),1);
    CHECK_EQ(plan.error,Error::None);CHECK_EQ(plan.intents.size(),2U);CHECK_EQ(f.core.canonical_state(),before);
    for(const auto& item:plan.intents){const auto result=f.apply(std::get<SubmitOrder>(item));CHECK_EQ(result.error,Error::None);}
    CHECK_EQ(f.core.state().orders.size(),2U);CHECK(f.core.invariant());
}
TEST_CASE(PLANNER_changed_quote_cancels_before_creating_and_pending_cancel_keeps_slot){
    Fixture f;const auto id=f.submit(intent(Side::Buy,Offset::Open,1));auto t=target(f.core);t.buy->price=498;
    auto plan=plan_quotes(f.core,t,1);CHECK_EQ(plan.intents.size(),1U);CHECK_EQ(std::get<RequestCancel>(plan.intents[0]).order,id);
    CHECK_EQ(f.apply(std::get<RequestCancel>(plan.intents[0])).error,Error::None);
    plan=plan_quotes(f.core,t,1);CHECK(plan.intents.empty());CHECK_EQ(plan.error,Error::None);
}
TEST_CASE(PLANNER_routes_yesterday_then_today_from_confirmed_positions){
    Fixture f(config({2,3,4,5}));auto t=target(f.core);auto plan=plan_quotes(f.core,t,1);
    CHECK_EQ(std::get<SubmitOrder>(plan.intents[0]).intent.offset,Offset::CloseYesterday);
    CHECK_EQ(std::get<SubmitOrder>(plan.intents[1]).intent.offset,Offset::CloseYesterday);
    Fixture today(config({0,0,2,0}));auto one=target(today.core);one.sell.reset();
    plan=plan_quotes(today.core,one,1);CHECK_EQ(std::get<SubmitOrder>(plan.intents[0]).intent.offset,Offset::CloseToday);
}
TEST_CASE(PLANNER_terminal_cancellation_allows_replacement_but_plan_cannot_bypass_funds){
    Fixture f;const auto id=f.submit(intent(Side::Buy,Offset::Open,1));
    CHECK_EQ(f.apply(TerminalReport{header(id),{TerminalKind::Canceled,0}}).error,Error::None);
    auto t=target(f.core);t.sell.reset();t.buy->price=498;
    const auto replacement=plan_quotes(f.core,t,1);CHECK_EQ(replacement.intents.size(),1U);
    CHECK_EQ(f.apply(std::get<SubmitOrder>(replacement.intents[0])).error,Error::None);
    auto c=config();c.risk.local_funds=1;Fixture poor(c);
    const auto plan=plan_quotes(poor.core,target(poor.core),1);CHECK_EQ(plan.error,Error::None);
    CHECK_EQ(poor.apply(std::get<SubmitOrder>(plan.intents[0])).error,Error::InsufficientFunds);
    CHECK(poor.core.state().orders.empty());
}
TEST_CASE(PLANNER_stale_generation_foreign_scope_crossed_quotes_and_kill_fail_closed){
    Fixture f;auto t=target(f.core);CHECK_EQ(plan_quotes(f.core,t,2).error,Error::Blocked);
    t.scope.account="other";CHECK_EQ(plan_quotes(f.core,t,1).error,Error::WrongIdentity);t=target(f.core);
    t.sell->price=499;CHECK_EQ(plan_quotes(f.core,t,1).error,Error::SelfTrade);
    CHECK_EQ(f.apply(KillRequested{}).error,Error::None);CHECK_EQ(plan_quotes(f.core,target(f.core),1).error,Error::Halted);
}
TEST_CASE(PLANNER_missing_trades_and_multiple_live_quotes_block_new_plans){
    Fixture f;const auto id=f.submit();CHECK_EQ(f.apply(TerminalReport{header(id),{TerminalKind::Canceled,1}}).error,Error::None);
    CHECK_EQ(plan_quotes(f.core,target(f.core),1).error,Error::Blocked);
    Fixture multiple;multiple.submit(intent(Side::Buy,Offset::Open,1));multiple.submit(intent(Side::Buy,Offset::Open,1,498));
    CHECK_EQ(plan_quotes(multiple.core,target(multiple.core),1).error,Error::Blocked);
}
