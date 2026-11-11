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
TEST_CASE(PLANNER_close_clipping_rounds_down_without_crossing_position_buckets){
    for(const bool buy:{false,true}){
        auto c=config(buy?std::array<Lots,4>{0,0,4,3}:std::array<Lots,4>{4,3,0,0});
        c.contract.min_lots=2;c.contract.lot_step=2;Fixture f(c);
        QuoteTarget t{f.core.scope(),c.strategy,1,session(),{}, {}};
        if(buy)t.buy=QuoteLevel{499,4};else t.sell=QuoteLevel{501,4};
        const auto before=f.core.canonical_state();const auto plan=plan_quotes(f.core,t,1);
        CHECK_EQ(plan.error,Error::None);CHECK_EQ(plan.intents.size(),1U);
        const auto& input=std::get<SubmitOrder>(plan.intents.at(0));
        CHECK_EQ(input.intent.offset,Offset::CloseYesterday);CHECK_EQ(input.intent.quantity,2);
        CHECK_EQ(c.contract.validate(input.intent),Error::None);CHECK_EQ(f.core.canonical_state(),before);
        CHECK_EQ(f.apply(input).error,Error::None);
        CHECK(plan_quotes(f.core,t,1).intents.empty());
    }
}
TEST_CASE(PLANNER_subminimum_yesterday_residual_does_not_open_or_skip_to_today){
    auto c=config({4,1,0,0});c.contract.min_lots=2;c.contract.lot_step=2;Fixture f(c);
    QuoteTarget t{f.core.scope(),c.strategy,1,session(),{},QuoteLevel{501,4}};
    const auto before=f.core.canonical_state();const auto plan=plan_quotes(f.core,t,1);
    CHECK_EQ(plan.error,Error::None);CHECK(plan.intents.empty());CHECK_EQ(f.core.canonical_state(),before);
}
TEST_CASE(PLANNER_partial_close_with_subminimum_remainder_requests_cancel_only){
    auto c=config({3,0,0,0});c.contract.min_lots=2;c.contract.lot_step=2;Fixture f(c);
    const auto id=f.submit(intent(Side::Sell,Offset::CloseToday,2,501));
    CHECK_EQ(f.apply(trade(id,"lot-residual",1,Side::Sell,Offset::CloseToday,501)).error,Error::None);
    QuoteTarget t{f.core.scope(),c.strategy,1,session(),{},QuoteLevel{501,4}};
    const auto plan=plan_quotes(f.core,t,1);CHECK_EQ(plan.error,Error::None);CHECK_EQ(plan.intents.size(),1U);
    CHECK_EQ(std::get<RequestCancel>(plan.intents.at(0)).order,id);
}
TEST_CASE(PLANNER_invalid_second_side_returns_no_partial_first_side_plan){
    for(const int fault:{0,1,2}){
        auto c=config();c.contract.tick_step=2;Fixture f(c);
        auto t=target(f.core);t.buy=QuoteLevel{498,2};t.sell=QuoteLevel{502,2};
        if(fault==0)t.sell->price=503;
        if(fault==1)t.sell->price=1002;
        if(fault==2)t.sell->quantity=11;
        const auto before=f.core.canonical_state();const auto plan=plan_quotes(f.core,t,1);
        CHECK_EQ(plan.error,fault==2?Error::QuantityRule:Error::PriceRule);
        CHECK(plan.intents.empty());CHECK_EQ(f.core.canonical_state(),before);
    }
}
TEST_CASE(PLANNER_closed_or_foreign_day_target_cannot_generate_submit_intents){
    Fixture f;auto t=target(f.core);t.session.second=12*3600;
    CHECK_EQ(plan_quotes(f.core,t,1).error,Error::ClosedSession);
    t.session=session();t.session.trading_day=next_day;
    CHECK_EQ(plan_quotes(f.core,t,1).error,Error::WrongTradingDay);
}
TEST_CASE(PLANNER_empty_target_can_withdraw_existing_orders_outside_session){
    Fixture f;const auto id=f.submit(intent(Side::Buy,Offset::Open,1));
    QuoteTarget t{f.core.scope(),f.core.config().strategy,1,session(),{},{}};t.session.second=12*3600;
    const auto p=plan_quotes(f.core,t,1);CHECK_EQ(p.error,Error::None);CHECK_EQ(p.intents.size(),1U);
    CHECK_EQ(std::get<RequestCancel>(p.intents.at(0)).order,id);
}
