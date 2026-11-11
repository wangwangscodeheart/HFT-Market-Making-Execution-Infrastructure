#include "fixture.hpp"
using namespace fixture;
TEST_CASE(P02_real_position_mapping_remains_blocked) {
    Fixture f; auto o=position_observation(); o.origin=EvidenceOrigin::ExternalUnconfigured;
    CHECK_EQ(f.apply(ObservePosition{o}).error,Error::Blocked); CHECK(f.core.state().external_positions.empty());
    o=position_observation(); o.shape="unverified.aggregate";
    CHECK_EQ(f.apply(ObservePosition{o}).error,Error::WrongIdentity); CHECK(f.core.state().external_positions.empty());
}

TEST_CASE(A01_core_rejects_bad_contract_calendar_tick_and_account) {
    for(int scenario=0;scenario<5;++scenario) {
        Fixture f; auto request=SubmitOrder{intent(),session()};
        if(scenario==0) request.intent.instrument="RB";
        if(scenario==1) request.intent.day=next_day;
        if(scenario==2) request.session.second=10*3600+20*60;
        if(scenario==3) request.intent.price=1001;
        if(scenario==4) request.intent.account="other";
        CHECK(f.apply(request).error!=Error::None); CHECK(f.core.state().orders.empty());
        CHECK_EQ(f.core.state().funds.view().local_required,0); CHECK_EQ(f.core.state().mode,CoreMode::Ready);
    }
}
TEST_CASE(A04_quantity_failure_has_no_order_or_reservation) {
    Fixture f; auto i=intent(); i.quantity=11;
    CHECK_EQ(f.apply(SubmitOrder{i,session()}).error,Error::QuantityRule);
    CHECK(f.core.state().orders.empty()); CHECK_EQ(f.core.state().funds.resources().size(),1U);
    CHECK_EQ(f.core.state().positions.frozen(),(std::array<Lots,4>{}));
}
TEST_CASE(A04_insufficient_close_rollback) {
    Fixture f(config({1,0,0,0}));
    CHECK_EQ(f.apply(SubmitOrder{intent(Side::Sell,Offset::CloseToday,2,501),session()}).error,Error::InsufficientClose);
    CHECK(f.core.state().orders.empty()); CHECK_EQ(f.core.state().positions.booked()[0],1);
    CHECK_EQ(f.core.state().positions.frozen()[0],0); CHECK_EQ(f.core.state().funds.resources().size(),1U);
}
TEST_CASE(A04_insufficient_funds_rolls_back_candidate_and_ID) {
    auto c=config(); c.risk.local_funds=200; Fixture f(c);
    CHECK_EQ(f.apply(SubmitOrder{intent(),session()}).error,Error::InsufficientFunds);
    CHECK(f.core.state().orders.empty()); CHECK_EQ(f.core.state().funds.view().local_required,0);
    CHECK_EQ(f.core.state().mode,CoreMode::Ready);
    const auto id=f.submit(intent(Side::Buy,Offset::Open,1)); CHECK_EQ(id.serial,1U);
}
TEST_CASE(A04_capacity_failure_preserves_existing_state) {
    auto c=config(); c.capacity.orders=1; Fixture f(c); const auto first=f.submit();
    const auto funds=f.core.state().funds.view().local_required;
    CHECK_EQ(f.apply(SubmitOrder{intent(),session()}).error,Error::Capacity);
    CHECK_EQ(f.core.state().orders.size(),1U); CHECK_EQ(f.order(first).parts().held(),3);
    CHECK_EQ(f.core.state().funds.view().local_required,funds);
}
TEST_CASE(A04_exception_seam_has_strong_transaction_rollback) {
    Fixture f(config({3,0,0,0})); f.core.fail_next_commit_for_test();
    const auto i=intent(Side::Sell,Offset::CloseToday,2,501);
    const auto failed=f.apply(SubmitOrder{i,session()});
    CHECK_EQ(failed.error,Error::InjectedFailure); CHECK(!failed.applied && !failed.order);
    CHECK(f.core.state().orders.empty()); CHECK_EQ(f.core.state().positions.frozen()[0],0);
    const auto id=f.submit(i); CHECK_EQ(id.serial,1U); CHECK_EQ(f.core.state().positions.frozen()[0],2);
}
TEST_CASE(A04_ID_exhaustion_does_not_leave_half_order) {
    auto c=config(); c.next_order=std::numeric_limits<std::uint64_t>::max(); Fixture f(c);
    const auto id=f.submit(intent(Side::Buy,Offset::Open,1)); CHECK_EQ(id.serial,c.next_order);
    CHECK_EQ(f.apply(SubmitOrder{intent(),session()}).error,Error::IdExhausted);
    CHECK_EQ(f.core.state().orders.size(),1U); CHECK_EQ(f.core.state().funds.resources().size(),2U);
}
TEST_CASE(A05_worst_case_interval_is_one_to_five) {
    Fixture f(config({2,0,0,0})); (void)f.submit();
    (void)f.submit(intent(Side::Sell,Offset::Open,1,501));
    const auto v=f.core.risk_view(); CHECK_EQ(v.booked_net,2); CHECK_EQ(v.min_net,1); CHECK_EQ(v.max_net,5);
    CHECK_EQ(v.gross_bound,6);
}
TEST_CASE(A05_opposite_quote_cannot_net_away_limit) {
    auto c=config({2,0,0,0}); c.risk.max_abs_net=4; Fixture f(c);
    (void)f.submit(intent(Side::Sell,Offset::Open,1,501));
    CHECK_EQ(f.apply(SubmitOrder{intent(),session()}).error,Error::PositionLimit);
    CHECK_EQ(f.core.state().orders.size(),1U); CHECK_EQ(f.core.risk_view().max_net,2);
}
TEST_CASE(A06_P01_six_effects_through_actual_core) {
    const std::array<std::pair<Side,Offset>,6> actions{{{Side::Buy,Offset::Open},{Side::Sell,Offset::Open},
        {Side::Sell,Offset::CloseToday},{Side::Sell,Offset::CloseYesterday},
        {Side::Buy,Offset::CloseToday},{Side::Buy,Offset::CloseYesterday}}};
    const std::array<std::array<Lots,4>,6> expected{{{6,6,7,8},{5,6,8,8},{4,6,7,8},{5,5,7,8},{5,6,6,8},{5,6,7,7}}};
    for(std::size_t n=0;n<actions.size();++n) {
        Fixture f(config({5,6,7,8})); const auto [side,offset]=actions[n]; const auto price=side==Side::Buy?499:501;
        const auto id=f.submit(intent(side,offset,1,price));
        const auto b=static_cast<std::size_t>(PositionLedger::affected_bucket(side,offset));
        CHECK_EQ(f.core.state().positions.frozen()[b],offset==Offset::Open?0:1);
        CHECK_EQ(f.apply(trade(id,"six-effects",1,side,offset,price)).error,Error::None);
        CHECK_EQ(f.core.state().positions.booked(),expected[n]); CHECK_EQ(f.core.state().positions.frozen()[b],0);
    }
}
TEST_CASE(A06_P01_close_freezes_compete_per_bucket) {
    Fixture f(config({2,3,0,0})); const auto ct=f.submit(intent(Side::Sell,Offset::CloseToday,2,501));
    CHECK_EQ(f.apply(SubmitOrder{intent(Side::Sell,Offset::CloseToday,1,502),session()}).error,Error::InsufficientClose);
    const auto cy=f.submit(intent(Side::Sell,Offset::CloseYesterday,3,502));
    CHECK_EQ(f.core.state().positions.frozen(),(std::array<Lots,4>{2,3,0,0}));
    CHECK_EQ(f.apply(TerminalReport{header(ct),{TerminalKind::Canceled,1}}).error,Error::None);
    CHECK_EQ(f.core.state().positions.frozen(),(std::array<Lots,4>{1,3,0,0}));
    CHECK_EQ(f.apply(trade(ct,"close-today",1,Side::Sell,Offset::CloseToday,501)).error,Error::None);
    CHECK_EQ(f.order(cy).parts().held(),3); CHECK_EQ(f.core.state().positions.booked()[0],1);
}
TEST_CASE(P02_external_and_settlement_never_overwrite_buckets) {
    Fixture f(config({2,3,4,5})); const auto before=f.core.state().positions.anchor();
    CHECK_EQ(f.apply(ObservePosition{position_observation(1,{99,99,99,99})}).error,Error::None);
    CHECK_EQ(f.apply(ObserveSettlement{settlement_observation(1,999)}).error,Error::None);
    CHECK_EQ(f.core.state().positions.booked(),before.booked); CHECK_EQ(f.core.state().positions.anchor(),before);
    CHECK_EQ(f.core.state().external_positions.size(),1U); CHECK_EQ(f.core.state().settlements.size(),1U);
}
TEST_CASE(P03_cross_day_freeze_stays_old_epoch_and_late_trade_books_once) {
    Fixture f(config({3,0,0,0})); const auto id=f.submit(intent(Side::Sell,Offset::CloseToday,2,501));
    CHECK_EQ(f.apply(DayBoundary{next_day}).error,Error::Blocked);
    CHECK_EQ(f.core.state().mode,CoreMode::BoundaryHalted); CHECK_EQ(f.core.state().positions.frozen()[0],2);
    const auto t=trade(id,"old-day-late",1,Side::Sell,Offset::CloseToday,501);
    CHECK_EQ(f.apply(t).error,Error::None); CHECK_EQ(f.apply(t).error,Error::Duplicate);
    CHECK_EQ(f.core.state().positions.anchor().day,day); CHECK_EQ(f.core.state().positions.anchor().epoch,1U);
    CHECK_EQ(f.core.state().positions.booked(),(std::array<Lots,4>{2,0,0,0}));
    CHECK_EQ(f.order(id).intent().offset,Offset::CloseToday); CHECK_EQ(f.core.state().positions.frozen()[0],1);
    CHECK_EQ(f.apply(SubmitOrder{intent(),session()}).error,Error::Halted);
}
TEST_CASE(P03_clean_boundary_still_requires_new_anchor_evidence) {
    Fixture f; CHECK_EQ(f.apply(DayBoundary{next_day}).error,Error::Blocked);
    CHECK_EQ(f.core.state().mode,CoreMode::AwaitDayEvidence);
    CHECK_EQ(f.apply(RequestFutureOperation{FutureOperation::ReconcileExternal}).error,Error::Blocked);
    CHECK_EQ(f.apply(SubmitOrder{intent(),session()}).error,Error::Halted);
}
TEST_CASE(RISK_reinstated_close_freeze_reports_deficit_without_losing_trade) {
    Fixture f(config({2,0,0,0})); const auto old=f.submit(intent(Side::Sell,Offset::CloseToday,2,501));
    CHECK_EQ(f.apply(TerminalReport{header(old),{TerminalKind::Rejected,0}}).error,Error::None);
    const auto newer=f.submit(intent(Side::Sell,Offset::CloseToday,2,502));
    CHECK_EQ(f.apply(trade(old,"contradiction",1,Side::Sell,Offset::CloseToday,501)).error,Error::EvidenceConflict);
    CHECK_EQ(f.order(old).booked(),1); CHECK_EQ(f.order(newer).parts().held(),2);
    CHECK_EQ(f.core.state().positions.booked()[0],1); CHECK_EQ(f.core.state().positions.frozen()[0],3);
    CHECK(f.core.state().risk_deficit); CHECK(f.core.risk_view().position_deficit);
}
TEST_CASE(RISK_stale_mark_rate_limit_and_gross_limit) {
    auto c=config(); c.risk.max_mark_age=1; Fixture stale(c);
    CHECK_EQ(stale.apply(ReferenceMark{500}).error,Error::None);
    CHECK_EQ(stale.apply(ObservePosition{position_observation()}).error,Error::None);
    CHECK_EQ(stale.apply(SubmitOrder{intent(),session()}).error,Error::StaleMark);
    c=config(); c.risk.order_rate=1; c.risk.rate_window=10; Fixture rate(c); (void)rate.submit();
    CHECK_EQ(rate.apply(SubmitOrder{intent(),session()}).error,Error::RateLimit);
    c=config(); c.risk.max_gross=2; Fixture gross(c);
    CHECK_EQ(gross.apply(SubmitOrder{intent(),session()}).error,Error::GrossLimit); CHECK(gross.core.state().orders.empty());
}
TEST_CASE(RISK_crossing_pending_cancel_and_unknown_orders_still_blocked) {
    Fixture f; const auto sell=f.submit(intent(Side::Sell,Offset::Open,1,501));
    CHECK(f.apply(RequestCancel{sell}).cancel);
    CHECK_EQ(f.apply(SubmitOrder{intent(Side::Buy,Offset::Open,1,501),session()}).error,Error::SelfTrade);
    CHECK_EQ(f.apply(OrderTimeout{sell}).error,Error::None);
    CHECK_EQ(f.apply(SubmitOrder{intent(),session()}).error,Error::Halted);
}
TEST_CASE(CAPACITY_execution_overflow_halts_and_preserves_unapplied_fact) {
    auto c=config(); c.capacity.trades=1; Fixture f(c); const auto id=f.submit();
    CHECK_EQ(f.apply(trade(id,"one")).error,Error::None); auto second=trade(id,"two");
    CHECK_EQ(f.apply(second).error,Error::Capacity); CHECK_EQ(f.order(id).booked(),1);
    CHECK_EQ(f.core.state().executions.records().size(),1U); CHECK(f.core.state().unknown_exposure);
    CHECK_EQ(f.core.state().quarantine.back().trade, std::optional<Trade>{second.trade});
    CHECK_EQ(f.apply(SubmitOrder{intent(),session()}).error,Error::Halted);
}
TEST_CASE(CAPACITY_quarantine_overflow_latches_loss_and_cannot_resume) {
    auto c=config(); c.capacity.quarantine=1; Fixture f(c); const auto id=f.submit();
    CHECK_EQ(f.apply(CumulativeReport{header(id),-1}).error,Error::InvalidInput);
    CHECK_EQ(f.apply(CumulativeReport{header(id),-2}).error,Error::InvalidInput);
    CHECK_EQ(f.core.state().quarantine.size(),1U); CHECK_EQ(f.core.state().evidence_loss_count,1U);
    CHECK_EQ(f.core.state().mode,CoreMode::CapacityHalted);
    CHECK_EQ(f.apply(RequestFutureOperation{FutureOperation::Reactivate}).error,Error::NotImplemented);
}
TEST_CASE(CAPACITY_evidence_streams_are_bounded) {
    auto c=config(); c.capacity.evidence_streams_per_order=1; Fixture f(c); const auto id=f.submit();
    CHECK_EQ(f.apply(CumulativeReport{header(id,1),1}).error,Error::None);
    auto other=header(id,2); other.stamp.source="fixture.other";
    CHECK_EQ(f.apply(CumulativeReport{other,2}).error,Error::Capacity); CHECK_EQ(f.order(id).lower_bound(),1);
    CHECK_EQ(f.order(id).stream_count(),1U);
}
TEST_CASE(BOUNDARY_Kill_blocks_new_but_processes_existing_trades) {
    Fixture f; const auto id=f.submit(); CHECK_EQ(f.apply(KillRequested{}).error,Error::None);
    CHECK_EQ(f.core.state().mode,CoreMode::Killed);
    CHECK_EQ(f.apply(SubmitOrder{intent(),session()}).error,Error::Halted);
    CHECK_EQ(f.apply(trade(id,"after-kill")).error,Error::None); CHECK_EQ(f.order(id).booked(),1);
    CHECK_EQ(f.core.state().mode,CoreMode::Killed);
}
TEST_CASE(BOUNDARY_future_capabilities_never_return_success) {
    Fixture f;
    for(const auto op:{FutureOperation::SendOrder,FutureOperation::EmergencyCancel,FutureOperation::JournalAppend,
        FutureOperation::ReconcileExternal,FutureOperation::CtpConnect,FutureOperation::LocalSimulator,
        FutureOperation::WebBridge,FutureOperation::ThreadRuntime,FutureOperation::RestoreSnapshot}) {
        const auto r=f.apply(RequestFutureOperation{op}); CHECK(!r.applied); CHECK(r.error!=Error::None);
        CHECK(CoreLoop::capability(op)!=Availability::Implemented);
    }
    CHECK(f.core.state().orders.empty());
}
TEST_CASE(BOUNDARY_external_CTP_evidence_is_blocked_not_trusted) {
    Fixture f; const auto id=f.submit(); auto h=header(id); h.stamp.origin=EvidenceOrigin::ExternalUnconfigured;
    CHECK_EQ(f.apply(CumulativeReport{h,1}).error,Error::Blocked); CHECK_EQ(f.order(id).lower_bound(),0);
    CHECK(!f.order(id).external_identity());
}
TEST_CASE(BOUNDARY_malformed_trade_and_unknown_identity_quarantine) {
    for(int scenario=0;scenario<4;++scenario) {
        Fixture f; const auto id=f.submit(); auto t=trade(id,"malformed");
        if(scenario==0) t.trade.quantity=-1;
        if(scenario==1) t.trade.price=0;
        if(scenario==2) t.trade.order={7,999};
        if(scenario==3) t.trade.key.trade_id=std::string(129,'x');
        CHECK(f.apply(t).error!=Error::None); CHECK(f.core.state().executions.records().empty());
        CHECK_EQ(f.order(id).booked(),0); CHECK(f.core.state().unknown_exposure);
    }
}
TEST_CASE(A01_actual_core_tick_and_quantity_step_reject) {
    auto c=config(); c.contract.tick_step=2; c.contract.min_lots=2; c.contract.lot_step=2;
    Fixture f(c);
    CHECK_EQ(f.apply(SubmitOrder{intent(Side::Buy,Offset::Open,2,499),session()}).error,Error::PriceRule);
    CHECK_EQ(f.apply(SubmitOrder{intent(Side::Buy,Offset::Open,3,500),session()}).error,Error::QuantityRule);
    CHECK(f.core.state().orders.empty()); CHECK_EQ(f.core.state().funds.view().local_required,0);
}
TEST_CASE(RISK_rate_window_expires_using_logical_time) {
    auto c=config(); c.risk.order_rate=1; c.risk.rate_window=10; Fixture f(c);
    (void)f.submit(intent(Side::Buy,Offset::Open,1));
    CHECK_EQ(f.apply(SubmitOrder{intent(Side::Buy,Offset::Open,1),session()}).error,Error::RateLimit);
    const auto r=f.core.step(SubmitOrder{intent(Side::Buy,Offset::Open,1),session()},11);
    CHECK_EQ(r.error,Error::None); CHECK_EQ(f.core.state().orders.size(),2U); CHECK(f.core.invariant());
}
TEST_CASE(RISK_economic_close_deficit_is_retained_not_clamped) {
    Fixture f(config({1,0,0,0})); const auto old=f.submit(intent(Side::Sell,Offset::CloseToday,1,501));
    CHECK_EQ(f.apply(TerminalReport{header(old),{TerminalKind::Rejected,0}}).error,Error::None);
    const auto newer=f.submit(intent(Side::Sell,Offset::CloseToday,1,502));
    CHECK_EQ(f.apply(trade(newer,"newer-close",1,Side::Sell,Offset::CloseToday,502)).error,Error::None);
    CHECK_EQ(f.apply(trade(old,"old-close",1,Side::Sell,Offset::CloseToday,501)).error,Error::EvidenceConflict);
    CHECK_EQ(f.core.state().positions.booked()[0],-1); CHECK_EQ(f.core.state().executions.records().size(),2U);
    CHECK(f.core.state().risk_deficit); CHECK(f.core.risk_view().position_deficit);
}
TEST_CASE(RISK_resource_aggregation_matches_per_order_scan_with_normal_and_exception_fills){
    Fixture f(config({20,0,0,0}));std::vector<OrderId> ids;
    for(int i=0;i<4;++i){ids.push_back(f.submit(intent(Side::Buy,Offset::Open,3)));ids.push_back(f.submit(intent(Side::Sell,Offset::CloseToday,3,501)));}
    for(std::size_t i=0;i<ids.size();++i){
        const auto& order=f.order(ids[i]);const auto side=order.intent().side;const auto offset=order.intent().offset;const auto price=order.intent().price;
        CHECK_EQ(f.apply(trade(ids[i],"aggregate-normal-"+std::to_string(i),1,side,offset,price)).error,Error::None);
        if(i%3==0)CHECK_EQ(f.apply(trade(ids[i],"aggregate-exception-"+std::to_string(i),3,side,offset,price)).error,Error::EvidenceConflict);
    }
    const auto& state=f.core.state();const auto& limits=f.core.config().risk;RiskEngine risk(limits);
    const auto anchor=state.funds.resources().begin()->first;
    const auto resources=risk.resources(state.positions,state.orders,state.executions,anchor);
    std::vector<FundResource> expected{{anchor,0,limits.anchor_margin,0}};
    for(const auto& [id,order]:state.orders){
        const auto normal=state.executions.normal_quantity(id),exception=state.executions.exception_quantity(id),held=order.parts().held();
        CHECK_EQ(order.booked(),normal);FundResource r{order.reservation(),0,0,0};
        r.position_margin=checked_mul_nonnegative(order.intent().offset==Offset::Open?checked_add(normal,exception):exception,limits.opening_margin_per_lot);
        if(order.intent().offset==Offset::Open)r.order_margin=checked_mul_nonnegative(held,limits.opening_margin_per_lot);
        r.charges=checked_mul_nonnegative(checked_add(checked_add(held,normal),exception),limits.fee_per_lot);expected.push_back(r);
    }
    CHECK_EQ(resources,expected);CHECK(f.core.invariant());
    const auto funds=state.funds.view().local_required;
    auto duplicate=trade(ids[0],"aggregate-exception-0",3);duplicate.channel=Channel::Query;
    CHECK_EQ(f.apply(duplicate).error,Error::Duplicate);CHECK_EQ(f.core.state().funds.view().local_required,funds);
}
TEST_CASE(TRANSACTION_aggregated_checks_keep_populated_ledger_unchanged_on_commit_failure){
    Fixture f;std::vector<OrderId> ids;
    for(int i=0;i<20;++i)ids.push_back(f.submit(intent(Side::Buy,Offset::Open,1)));
    for(std::size_t i=0;i<ids.size();++i)CHECK_EQ(f.apply(trade(ids[i],"aggregation-rollback-"+std::to_string(i))).error,Error::None);
    const auto before=f.core.canonical_state();f.core.fail_next_commit_for_test();
    CHECK_EQ(f.core.step(ReferenceMark{501},++f.time).error,Error::InjectedFailure);CHECK_EQ(f.core.canonical_state(),before);CHECK(f.core.invariant());
    CHECK_EQ(f.apply(ReferenceMark{501}).error,Error::None);CHECK_EQ(f.core.state().positions.booked()[0],20);
}
