#include "fixture.hpp"
#include <algorithm>
#include <iostream>
using namespace fixture;

TEST_CASE(O01_untrusted_negative_and_over_Q_do_not_promote_R) {
    for(int scenario=0;scenario<3;++scenario) {
        Fixture f; const auto id=f.submit(); auto h=header(id);
        Lots q=2; if(scenario==0) h.stamp.origin=EvidenceOrigin::Unverified;
        if(scenario==1) q=-1;
        if(scenario==2) q=4;
        const auto r=f.apply(CumulativeReport{h,q}); CHECK(r.error!=Error::None);
        CHECK_EQ(f.order(id).lower_bound(),0); CHECK_EQ(f.order(id).booked(),0);
        CHECK_EQ(f.core.state().quarantine.size(),1U); CHECK_EQ(f.order(id).parts().held(),3);
        CHECK(f.core.state().mode!=CoreMode::Ready);
    }
}
TEST_CASE(O01_identity_scope_is_checked_before_evidence) {
    for(int scenario=0;scenario<3;++scenario) {
        Fixture f; const auto id=f.submit(); auto h=header(id);
        if(scenario==0) h.account="other";
        if(scenario==1) h.day=next_day;
        if(scenario==2) h.order={99,3};
        CHECK(f.apply(CumulativeReport{h,2}).error!=Error::None); CHECK_EQ(f.order(id).lower_bound(),0);
        CHECK(f.core.state().unknown_exposure);
    }
}
TEST_CASE(O02_all_720_cumulative_trade_orderings) {
    std::array<int,6> permutation{0,1,2,3,4,5}; std::size_t count=0;
    do {
        Fixture f; const auto id=f.submit();
        const std::array<BusinessEvent,6> inputs{CumulativeReport{header(id),1},CumulativeReport{header(id),2},
            trade(id,"t1"),trade(id,"t2"),trade(id,"t3"),TerminalReport{header(id),{TerminalKind::Filled,3}}};
        for(const auto n:permutation) {
            CHECK_EQ(f.apply(inputs[static_cast<std::size_t>(n)]).error,Error::None);
            const auto p=f.order(id).parts(); CHECK_EQ(p.total(),3); CHECK_EQ(p.released,0);
        }
        CHECK_EQ(f.order(id).booked(),3); CHECK_EQ(f.order(id).parts().suspense,0);
        CHECK_EQ(f.order(id).consistency(),Consistency::InSync); CHECK_EQ(f.core.state().positions.booked()[0],3);
        ++count;
    } while(std::next_permutation(permutation.begin(),permutation.end()));
    CHECK_EQ(count,720U); std::cout<<"DETAIL O02 permutations="<<count<<'\n';
}
TEST_CASE(O02_R_greater_and_B_greater_are_not_conflicts) {
    Fixture f; const auto id=f.submit();
    CHECK_EQ(f.apply(CumulativeReport{header(id),2}).error,Error::None);
    CHECK_EQ(f.order(id).parts(),(QuantityParts{0,2,1,0,0}));
    CHECK_EQ(f.apply(trade(id,"a")).error,Error::None);
    CHECK_EQ(f.order(id).parts(),(QuantityParts{1,1,1,0,0}));
    CHECK_EQ(f.apply(trade(id,"b",2)).error,Error::None);
    CHECK_EQ(f.order(id).booked(),3); CHECK_EQ(f.order(id).lower_bound(),2);
    CHECK_EQ(f.order(id).consistency(),Consistency::InSync);
}
TEST_CASE(O03_A07_cancel_exact_and_late_trade_release_only_one) {
    Fixture f; const auto id=f.submit();
    CHECK_EQ(f.apply(trade(id,"t1")).error,Error::None);
    const auto cancel=f.apply(RequestCancel{id}); CHECK(cancel.cancel);
    CHECK_EQ(f.order(id).parts().held(),2);
    CHECK_EQ(f.apply(TerminalReport{header(id),{TerminalKind::Canceled,2}}).error,Error::None);
    CHECK_EQ(f.order(id).parts(),(QuantityParts{1,1,0,1,0}));
    const auto required=f.core.state().funds.view().local_required;
    const auto t2=trade(id,"t2"); CHECK_EQ(f.apply(t2).error,Error::None);
    CHECK_EQ(f.order(id).parts(),(QuantityParts{2,0,0,1,0}));
    CHECK_EQ(f.apply(t2).error,Error::Duplicate);
    CHECK_EQ(f.core.state().positions.booked()[0],2); CHECK_EQ(f.core.state().funds.view().local_required,required);
    CHECK_EQ(f.order(id).execution(),ExecutionState::Canceled);
}
TEST_CASE(O04_F_less_than_B_keeps_booked_facts_and_halts) {
    Fixture f; const auto id=f.submit(); CHECK_EQ(f.apply(trade(id,"t",2)).error,Error::None);
    CHECK_EQ(f.apply(TerminalReport{header(id),{TerminalKind::Canceled,1}}).error,Error::EvidenceConflict);
    CHECK_EQ(f.order(id).booked(),2); CHECK_EQ(f.order(id).parts().held(),1);
    CHECK_EQ(f.core.state().positions.booked()[0],2); CHECK_EQ(f.order(id).consistency(),Consistency::Conflict);
    CHECK_EQ(f.apply(SubmitOrder{intent(),session()}).error,Error::Halted);
}
TEST_CASE(O04_late_trade_after_reject_restores_conservative_hold) {
    Fixture f; const auto id=f.submit();
    CHECK_EQ(f.apply(TerminalReport{header(id),{TerminalKind::Rejected,0}}).error,Error::None);
    CHECK_EQ(f.order(id).parts().released,3); CHECK_EQ(f.core.state().funds.view().local_required,0);
    CHECK_EQ(f.apply(trade(id,"late")).error,Error::EvidenceConflict);
    CHECK_EQ(f.order(id).booked(),1); CHECK_EQ(f.order(id).parts().held(),2);
    CHECK_EQ(f.order(id).parts().released,0); CHECK_EQ(f.core.state().positions.booked()[0],1);
    CHECK_EQ(f.core.state().funds.view().local_required,306);
}
TEST_CASE(O04_terminal_exact_contradiction_cannot_release_twice) {
    Fixture f; const auto id=f.submit();
    CHECK_EQ(f.apply(TerminalReport{header(id),{TerminalKind::Canceled,1}}).error,Error::None);
    CHECK_EQ(f.apply(TerminalReport{header(id),{TerminalKind::Canceled,2}}).error,Error::EvidenceConflict);
    CHECK_EQ(f.order(id).parts().released,0); CHECK_EQ(f.order(id).parts().held(),3);
    CHECK_EQ(f.core.state().quarantine.back().reported_quantity,2);
}
TEST_CASE(O05_query_push_share_economic_dedup) {
    Fixture f; const auto id=f.submit(); const auto push=trade(id,"same");
    auto query=push; query.channel=Channel::Query; query.stamp=stamp(123,"fixture.query");
    CHECK_EQ(f.apply(push).error,Error::None); CHECK_EQ(f.apply(query).error,Error::Duplicate);
    CHECK_EQ(f.order(id).booked(),1); CHECK_EQ(f.core.state().executions.records().size(),1U);
}
TEST_CASE(O06_same_key_changed_payload_preserves_original) {
    for(int variant=0;variant<3;++variant) {
        Fixture f; const auto id=f.submit(); const auto t=trade(id,"same");
        CHECK_EQ(f.apply(t).error,Error::None); auto changed=t;
        if(variant==0) changed.trade.quantity=2;
        if(variant==1) changed.trade.price=498;
        if(variant==2) changed.trade.order={7,999};
        CHECK_EQ(f.apply(changed).error,Error::EvidenceConflict);
        CHECK_EQ(f.core.state().executions.records().at(t.trade.key).trade,t.trade);
        CHECK_EQ(f.order(id).booked(),1); CHECK_EQ(f.order(id).consistency(),Consistency::Conflict);
        CHECK(f.core.state().quarantine.back().trade);
    }
}
TEST_CASE(O07_A08_overfill_entire_trade_to_X_and_dedup) {
    Fixture f; const auto id=f.submit(); CHECK_EQ(f.apply(trade(id,"normal",2)).error,Error::None);
    auto excess=trade(id,"excess",2);
    CHECK_EQ(f.apply(excess).error,Error::EvidenceConflict);
    CHECK_EQ(f.order(id).booked(),2); CHECK_EQ(f.core.state().executions.exception_quantity(id),2);
    CHECK_EQ(f.core.state().positions.booked()[0],2); CHECK_EQ(f.core.risk_view().exception_lots,2);
    CHECK_EQ(f.order(id).parts().total(),3);
    const auto funds=f.core.state().funds.view().local_required;
    excess.channel=Channel::Query; CHECK_EQ(f.apply(excess).error,Error::Duplicate);
    CHECK_EQ(f.core.state().executions.exception_quantity(id),2);
    CHECK_EQ(f.core.state().funds.view().local_required,funds);
}
TEST_CASE(O08_no_exact_terminal_holds_V_and_H) {
    Fixture f; const auto id=f.submit(); CHECK_EQ(f.apply(CumulativeReport{header(id),1}).error,Error::None);
    CHECK_EQ(f.apply(TerminalReport{header(id),{TerminalKind::Canceled,{}}}).error,Error::None);
    CHECK_EQ(f.order(id).parts(),(QuantityParts{0,1,0,0,2}));
    CHECK_EQ(f.core.state().funds.view().local_required,306);
    CHECK_EQ(f.apply(trade(id,"late")).error,Error::None);
    CHECK_EQ(f.order(id).parts(),(QuantityParts{1,0,0,0,2}));
    CHECK_EQ(f.order(id).consistency(),Consistency::Unreconciled);
}
TEST_CASE(O09_comparable_newer_regression_conflicts) {
    Fixture f; const auto id=f.submit();
    CHECK_EQ(f.apply(CumulativeReport{header(id,2),2}).error,Error::None);
    CHECK_EQ(f.apply(CumulativeReport{header(id,3),1}).error,Error::EvidenceConflict);
    CHECK_EQ(f.order(id).lower_bound(),2); CHECK_EQ(f.order(id).parts().released,0);
}
TEST_CASE(O09_older_smaller_and_distinct_epoch_do_not_regress) {
    Fixture f; const auto id=f.submit();
    CHECK_EQ(f.apply(CumulativeReport{header(id,2),2}).error,Error::None);
    CHECK_EQ(f.apply(CumulativeReport{header(id,1),1}).error,Error::None);
    auto h=header(id,3); h.stamp.epoch=2;
    CHECK_EQ(f.apply(CumulativeReport{h,1}).error,Error::None); CHECK_EQ(f.order(id).lower_bound(),2);
    CHECK_EQ(f.order(id).consistency(),Consistency::MissingTrades);
}
TEST_CASE(OMS_cancel_reject_does_not_reject_order) {
    Fixture f; const auto id=f.submit(); CHECK_EQ(f.apply(WorkingReport{header(id)}).error,Error::None);
    const auto c=f.apply(RequestCancel{id}); CHECK(c.cancel);
    CHECK_EQ(f.apply(CancelReport{header(id),*c.cancel,CancelState::Rejected}).error,Error::None);
    CHECK_EQ(f.order(id).execution(),ExecutionState::Working); CHECK_EQ(f.order(id).parts().held(),3);
    CHECK_EQ(f.apply(trade(id,"after-reject")).error,Error::None); CHECK_EQ(f.order(id).booked(),1);
}
TEST_CASE(OMS_cancel_success_without_F_does_not_release) {
    Fixture f; const auto id=f.submit(); const auto c=f.apply(RequestCancel{id}); CHECK(c.cancel);
    CHECK_EQ(f.apply(CancelReport{header(id),*c.cancel,CancelState::Succeeded}).error,Error::None);
    CHECK_EQ(f.order(id).parts().held(),3); CHECK_EQ(f.order(id).parts().released,0);
}
TEST_CASE(OMS_working_late_cannot_reopen_terminal) {
    Fixture f; const auto id=f.submit();
    CHECK_EQ(f.apply(TerminalReport{header(id),{TerminalKind::Canceled,0}}).error,Error::None);
    CHECK_EQ(f.apply(WorkingReport{header(id)}).error,Error::None);
    CHECK_EQ(f.order(id).execution(),ExecutionState::Canceled); CHECK_EQ(f.order(id).parts().released,3);
}
TEST_CASE(OMS_timeout_holds_risk_and_never_resubmits) {
    Fixture f; const auto id=f.submit(); CHECK_EQ(f.apply(OrderTimeout{id}).error,Error::None);
    CHECK_EQ(f.order(id).consistency(),Consistency::Unreconciled); CHECK_EQ(f.order(id).parts().held(),3);
    CHECK_EQ(f.core.state().orders.size(),1U); CHECK_EQ(f.order(id).send(),SendState::NotImplemented);
}
TEST_CASE(OMS_unknown_cancel_stops_new_intents_but_books_late_trade) {
    Fixture f; const auto id=f.submit(); const auto c=f.apply(RequestCancel{id}); CHECK(c.cancel);
    CHECK_EQ(f.apply(CancelReport{header(id),*c.cancel,CancelState::Unknown}).error,Error::None);
    CHECK_EQ(f.order(id).cancel(),CancelState::Unknown); CHECK_EQ(f.order(id).parts().held(),3);
    CHECK_EQ(f.apply(SubmitOrder{intent(),session()}).error,Error::Halted);
    CHECK_EQ(f.apply(trade(id,"after-unknown-cancel")).error,Error::None); CHECK_EQ(f.order(id).booked(),1);
    CHECK_EQ(f.apply(CancelReport{header(id),*c.cancel,CancelState::Succeeded}).error,Error::None);
    CHECK_EQ(f.core.state().mode,CoreMode::ConflictHalted); CHECK_EQ(f.order(id).parts().released,0);
}
TEST_CASE(OMS_late_old_cancel_report_does_not_overwrite_new_action) {
    Fixture f; const auto id=f.submit(); const auto first=f.apply(RequestCancel{id}); CHECK(first.cancel);
    CHECK_EQ(f.apply(CancelReport{header(id),*first.cancel,CancelState::Rejected}).error,Error::None);
    const auto second=f.apply(RequestCancel{id}); CHECK(second.cancel && second.cancel!=first.cancel);
    CHECK_EQ(f.apply(CancelReport{header(id),*first.cancel,CancelState::Rejected}).error,Error::Duplicate);
    CHECK_EQ(f.order(id).cancel_action(),second.cancel); CHECK_EQ(f.order(id).cancel(),CancelState::Requested);
    CHECK_EQ(f.order(id).cancel_action_count(),2U); CHECK_EQ(f.core.state().mode,CoreMode::Ready);
}
TEST_CASE(OMS_cancel_action_capacity_rejects_without_half_action) {
    auto c=config(); c.capacity.cancel_actions_per_order=1; Fixture f(c); const auto id=f.submit();
    const auto first=f.apply(RequestCancel{id}); CHECK(first.cancel);
    CHECK_EQ(f.apply(CancelReport{header(id),*first.cancel,CancelState::Rejected}).error,Error::None);
    CHECK_EQ(f.apply(RequestCancel{id}).error,Error::Capacity);
    CHECK_EQ(f.order(id).cancel_action(),first.cancel); CHECK_EQ(f.order(id).cancel_action_count(),1U);
}
TEST_CASE(OMS_reliable_acceptance_conflicts_with_NotSent_or_reject) {
    for(const auto kind:{TerminalKind::NotSent,TerminalKind::Rejected}) {
        for(bool acceptance_first:{false,true}) {
            Fixture f; const auto id=f.submit();
            if(acceptance_first) CHECK_EQ(f.apply(WorkingReport{header(id)}).error,Error::None);
            const auto terminal=f.apply(TerminalReport{header(id),{kind,0}});
            if(acceptance_first) CHECK_EQ(terminal.error,Error::EvidenceConflict);
            else {
                CHECK_EQ(terminal.error,Error::None);
                CHECK_EQ(f.apply(WorkingReport{header(id)}).error,Error::EvidenceConflict);
            }
            CHECK_EQ(f.order(id).parts().released,0); CHECK_EQ(f.order(id).parts().held(),3);
            CHECK_EQ(f.order(id).consistency(),Consistency::Conflict);
        }
    }
}
TEST_CASE(OMS_cumulative_partial_cannot_regress_to_working) {
    Fixture f; const auto id=f.submit(); CHECK_EQ(f.apply(CumulativeReport{header(id),1}).error,Error::None);
    CHECK_EQ(f.order(id).execution(),ExecutionState::PartiallyFilled);
    CHECK_EQ(f.apply(WorkingReport{header(id)}).error,Error::None);
    CHECK_EQ(f.order(id).execution(),ExecutionState::PartiallyFilled);
}
TEST_CASE(OMS_full_terminal_cannot_regress_to_canceled_from_late_snapshot) {
    Fixture f; const auto id=f.submit(); CHECK_EQ(f.apply(trade(id,"all",3)).error,Error::None);
    CHECK_EQ(f.order(id).execution(),ExecutionState::Filled);
    CHECK_EQ(f.apply(TerminalReport{header(id),{TerminalKind::Canceled,3}}).error,Error::None);
    CHECK_EQ(f.order(id).execution(),ExecutionState::Filled); CHECK_EQ(f.order(id).parts().released,0);
}
TEST_CASE(OMS_invalid_terminal_enum_cannot_release) {
    Fixture f; const auto id=f.submit();
    CHECK_EQ(f.apply(TerminalReport{header(id),{static_cast<TerminalKind>(99),0}}).error,Error::InvalidInput);
    CHECK_EQ(f.order(id).parts().released,0); CHECK_EQ(f.order(id).lower_bound(),0);
}
TEST_CASE(O09_equal_version_changed_quantity_conflicts) {
    Fixture f; const auto id=f.submit();
    CHECK_EQ(f.apply(CumulativeReport{header(id,2),1}).error,Error::None);
    CHECK_EQ(f.apply(CumulativeReport{header(id,2),2}).error,Error::EvidenceConflict);
    CHECK_EQ(f.order(id).lower_bound(),2);
}
TEST_CASE(O05_identical_economic_content_with_different_trade_id_is_distinct) {
    Fixture f; const auto id=f.submit(); CHECK_EQ(f.apply(trade(id,"id-a")).error,Error::None);
    CHECK_EQ(f.apply(trade(id,"id-b")).error,Error::None); CHECK_EQ(f.order(id).booked(),2);
}
