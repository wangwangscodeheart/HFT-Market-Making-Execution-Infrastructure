# M1A 自动化测试清单

本清单由源码声明、已运行的测试清单和 CTest JUnit 三方核对生成。
每例在 Debug、Release、ASan/UBSan 下均实际通过；没有 skip。LSan 未通过验收。
完整逐例检查次数见 evidence/*-tests.log；每种构建均包含 O02 的 720 种排列。

| 测试 | 架构契约/范围 | 源文件位置 | 最终三构建结果 |
|---|---|---|---|
| `A01_actual_core_tick_and_quantity_step_reject` | A01 | `tests/core_tests.cpp:204` | PASS / PASS / PASS |
| `A01_bad_calendar_configuration_rejected` | A01 | `tests/types_tests.cpp:26` | PASS / PASS / PASS |
| `A01_calendar_night_mapping_and_breaks` | A01 | `tests/types_tests.cpp:17` | PASS / PASS / PASS |
| `A01_checked_arithmetic_boundaries` | A01 | `tests/types_tests.cpp:46` | PASS / PASS / PASS |
| `A01_contract_tick_quantity_and_day_rules` | A01 | `tests/types_tests.cpp:30` | PASS / PASS / PASS |
| `A01_core_rejects_bad_contract_calendar_tick_and_account` | A01 | `tests/core_tests.cpp:10` | PASS / PASS / PASS |
| `A01_dates_validate_gregorian` | A01 | `tests/types_tests.cpp:13` | PASS / PASS / PASS |
| `A01_identity_types_and_exhaustion` | A01 | `tests/types_tests.cpp:6` | PASS / PASS / PASS |
| `A01_unsupported_real_profile_cannot_boot` | A01 | `tests/types_tests.cpp:41` | PASS / PASS / PASS |
| `A04_ID_exhaustion_does_not_leave_half_order` | A04 | `tests/core_tests.cpp:56` | PASS / PASS / PASS |
| `A04_capacity_failure_preserves_existing_state` | A04 | `tests/core_tests.cpp:41` | PASS / PASS / PASS |
| `A04_exception_seam_has_strong_transaction_rollback` | A04 | `tests/core_tests.cpp:48` | PASS / PASS / PASS |
| `A04_insufficient_close_rollback` | A04 | `tests/core_tests.cpp:28` | PASS / PASS / PASS |
| `A04_insufficient_funds_rolls_back_candidate_and_ID` | A04 | `tests/core_tests.cpp:34` | PASS / PASS / PASS |
| `A04_quantity_failure_has_no_order_or_reservation` | A04 | `tests/core_tests.cpp:22` | PASS / PASS / PASS |
| `A05_opposite_quote_cannot_net_away_limit` | A05 | `tests/core_tests.cpp:68` | PASS / PASS / PASS |
| `A05_worst_case_interval_is_one_to_five` | A05 | `tests/core_tests.cpp:62` | PASS / PASS / PASS |
| `A06_P01_close_freezes_compete_per_bucket` | A06、P01 | `tests/core_tests.cpp:88` | PASS / PASS / PASS |
| `A06_P01_six_effects_through_actual_core` | A06、P01 | `tests/core_tests.cpp:74` | PASS / PASS / PASS |
| `BOUNDARY_Kill_blocks_new_but_processes_existing_trades` | M1A 基础/边界 | `tests/core_tests.cpp:171` | PASS / PASS / PASS |
| `BOUNDARY_external_CTP_evidence_is_blocked_not_trusted` | M1A 基础/边界 | `tests/core_tests.cpp:188` | PASS / PASS / PASS |
| `BOUNDARY_future_capabilities_never_return_success` | M1A 基础/边界 | `tests/core_tests.cpp:178` | PASS / PASS / PASS |
| `BOUNDARY_malformed_trade_and_unknown_identity_quarantine` | M1A 基础/边界 | `tests/core_tests.cpp:193` | PASS / PASS / PASS |
| `CAPACITY_evidence_streams_are_bounded` | M1A 基础/边界 | `tests/core_tests.cpp:164` | PASS / PASS / PASS |
| `CAPACITY_execution_overflow_halts_and_preserves_unapplied_fact` | M1A 基础/边界 | `tests/core_tests.cpp:148` | PASS / PASS / PASS |
| `CAPACITY_quarantine_overflow_latches_loss_and_cannot_resume` | M1A 基础/边界 | `tests/core_tests.cpp:156` | PASS / PASS / PASS |
| `FUNDS_duplicate_resource_transfer_cannot_double_proof` | M1A 基础/边界 | `tests/funds_tests.cpp:81` | PASS / PASS / PASS |
| `FUNDS_invalid_coverage_status_cannot_activate_money` | M1A 基础/边界 | `tests/funds_tests.cpp:86` | PASS / PASS / PASS |
| `FUNDS_same_snapshot_changed_payload_conflict_preserves_baseline` | M1A 基础/边界 | `tests/funds_tests.cpp:75` | PASS / PASS / PASS |
| `HARNESS_bounded_schedule_and_past_time_rejection` | M1A 基础/边界 | `tests/harness_tests.cpp:24` | PASS / PASS / PASS |
| `HARNESS_core_rejects_clock_regression_without_financial_change` | M1A 基础/边界 | `tests/harness_tests.cpp:31` | PASS / PASS / PASS |
| `HARNESS_equal_time_preserves_insertion_order` | M1A 基础/边界 | `tests/harness_tests.cpp:4` | PASS / PASS / PASS |
| `HARNESS_failed_evidence_input_is_retained_with_result` | M1A 基础/边界 | `tests/harness_tests.cpp:36` | PASS / PASS / PASS |
| `HARNESS_same_ordered_inputs_same_business_states` | M1A 基础/边界 | `tests/harness_tests.cpp:11` | PASS / PASS / PASS |
| `HARNESS_summary_distinguishes_hidden_cumulative_versions` | M1A 基础/边界 | `tests/harness_tests.cpp:43` | PASS / PASS / PASS |
| `O01_identity_scope_is_checked_before_evidence` | O01 | `tests/oms_tests.cpp:18` | PASS / PASS / PASS |
| `O01_untrusted_negative_and_over_Q_do_not_promote_R` | O01 | `tests/oms_tests.cpp:6` | PASS / PASS / PASS |
| `O02_R_greater_and_B_greater_are_not_conflicts` | O02 | `tests/oms_tests.cpp:44` | PASS / PASS / PASS |
| `O02_all_720_cumulative_trade_orderings` | O02 | `tests/oms_tests.cpp:28` | PASS / PASS / PASS |
| `O03_A07_cancel_exact_and_late_trade_release_only_one` | A07、O03 | `tests/oms_tests.cpp:54` | PASS / PASS / PASS |
| `O04_F_less_than_B_keeps_booked_facts_and_halts` | O04 | `tests/oms_tests.cpp:68` | PASS / PASS / PASS |
| `O04_late_trade_after_reject_restores_conservative_hold` | O04 | `tests/oms_tests.cpp:75` | PASS / PASS / PASS |
| `O04_terminal_exact_contradiction_cannot_release_twice` | O04 | `tests/oms_tests.cpp:84` | PASS / PASS / PASS |
| `O05_identical_economic_content_with_different_trade_id_is_distinct` | O05 | `tests/oms_tests.cpp:231` | PASS / PASS / PASS |
| `O05_query_push_share_economic_dedup` | O05 | `tests/oms_tests.cpp:91` | PASS / PASS / PASS |
| `O06_same_key_changed_payload_preserves_original` | O06 | `tests/oms_tests.cpp:97` | PASS / PASS / PASS |
| `O07_A08_overfill_entire_trade_to_X_and_dedup` | A08、O07 | `tests/oms_tests.cpp:110` | PASS / PASS / PASS |
| `O08_no_exact_terminal_holds_V_and_H` | O08 | `tests/oms_tests.cpp:122` | PASS / PASS / PASS |
| `O09_comparable_newer_regression_conflicts` | O09 | `tests/oms_tests.cpp:131` | PASS / PASS / PASS |
| `O09_equal_version_changed_quantity_conflicts` | O09 | `tests/oms_tests.cpp:225` | PASS / PASS / PASS |
| `O09_older_smaller_and_distinct_epoch_do_not_regress` | O09 | `tests/oms_tests.cpp:137` | PASS / PASS / PASS |
| `OMS_cancel_action_capacity_rejects_without_half_action` | M1A 基础/边界 | `tests/oms_tests.cpp:185` | PASS / PASS / PASS |
| `OMS_cancel_reject_does_not_reject_order` | M1A 基础/边界 | `tests/oms_tests.cpp:145` | PASS / PASS / PASS |
| `OMS_cancel_success_without_F_does_not_release` | M1A 基础/边界 | `tests/oms_tests.cpp:152` | PASS / PASS / PASS |
| `OMS_cumulative_partial_cannot_regress_to_working` | M1A 基础/边界 | `tests/oms_tests.cpp:208` | PASS / PASS / PASS |
| `OMS_full_terminal_cannot_regress_to_canceled_from_late_snapshot` | M1A 基础/边界 | `tests/oms_tests.cpp:214` | PASS / PASS / PASS |
| `OMS_invalid_terminal_enum_cannot_release` | M1A 基础/边界 | `tests/oms_tests.cpp:220` | PASS / PASS / PASS |
| `OMS_late_old_cancel_report_does_not_overwrite_new_action` | M1A 基础/边界 | `tests/oms_tests.cpp:177` | PASS / PASS / PASS |
| `OMS_reliable_acceptance_conflicts_with_NotSent_or_reject` | M1A 基础/边界 | `tests/oms_tests.cpp:192` | PASS / PASS / PASS |
| `OMS_timeout_holds_risk_and_never_resubmits` | M1A 基础/边界 | `tests/oms_tests.cpp:163` | PASS / PASS / PASS |
| `OMS_unknown_cancel_stops_new_intents_but_books_late_trade` | M1A 基础/边界 | `tests/oms_tests.cpp:168` | PASS / PASS / PASS |
| `OMS_working_late_cannot_reopen_terminal` | M1A 基础/边界 | `tests/oms_tests.cpp:157` | PASS / PASS / PASS |
| `P01_anchor_is_not_mutated_by_booking` | P01 | `tests/types_tests.cpp:66` | PASS / PASS / PASS |
| `P01_invalid_side_offset_and_bucket_fail_without_mutation` | P01 | `tests/types_tests.cpp:73` | PASS / PASS / PASS |
| `P01_six_position_effects` | P01 | `tests/types_tests.cpp:56` | PASS / PASS / PASS |
| `P02_external_and_settlement_never_overwrite_buckets` | P02 | `tests/core_tests.cpp:98` | PASS / PASS / PASS |
| `P02_real_position_mapping_remains_blocked` | P02 | `tests/core_tests.cpp:3` | PASS / PASS / PASS |
| `P03_clean_boundary_still_requires_new_anchor_evidence` | P03 | `tests/core_tests.cpp:116` | PASS / PASS / PASS |
| `P03_cross_day_freeze_stays_old_epoch_and_late_trade_books_once` | P03 | `tests/core_tests.cpp:105` | PASS / PASS / PASS |
| `P04_core_insufficient_external_budget_rejects_atomic_reservation` | P04 | `tests/funds_tests.cpp:31` | PASS / PASS / PASS |
| `P04_duplicate_or_excess_coverage_rejected_atomically` | P04 | `tests/funds_tests.cpp:25` | PASS / PASS / PASS |
| `P04_full_and_partial_proven_coverage_subtract_only_difference` | P04 | `tests/funds_tests.cpp:12` | PASS / PASS / PASS |
| `P04_local_cap_is_independent_of_external_available` | P04 | `tests/funds_tests.cpp:20` | PASS / PASS / PASS |
| `P04_one_external_deduction_cannot_cover_two_resources` | P04 | `tests/funds_tests.cpp:91` | PASS / PASS / PASS |
| `P05_unknown_and_excluded_coverage_are_zero` | P05 | `tests/funds_tests.cpp:36` | PASS / PASS / PASS |
| `P05_untrusted_and_wrong_anchor_cannot_activate_coverage` | P05 | `tests/funds_tests.cpp:44` | PASS / PASS / PASS |
| `P05_wrong_currency_and_unbounded_query_window_rejected` | P05 | `tests/funds_tests.cpp:97` | PASS / PASS / PASS |
| `P06_cancel_release_does_not_credit_external_available` | P06 | `tests/funds_tests.cpp:69` | PASS / PASS / PASS |
| `P06_post_anchor_fees_are_not_covered_by_margin_proof` | P06 | `tests/funds_tests.cpp:61` | PASS / PASS / PASS |
| `P06_transfer_keeps_one_resource_and_no_fabricated_money` | P06 | `tests/funds_tests.cpp:50` | PASS / PASS / PASS |
| `RISK_crossing_pending_cancel_and_unknown_orders_still_blocked` | M1A 基础/边界 | `tests/core_tests.cpp:141` | PASS / PASS / PASS |
| `RISK_economic_close_deficit_is_retained_not_clamped` | M1A 基础/边界 | `tests/core_tests.cpp:218` | PASS / PASS / PASS |
| `RISK_rate_window_expires_using_logical_time` | M1A 基础/边界 | `tests/core_tests.cpp:211` | PASS / PASS / PASS |
| `RISK_reinstated_close_freeze_reports_deficit_without_losing_trade` | M1A 基础/边界 | `tests/core_tests.cpp:122` | PASS / PASS / PASS |
| `RISK_stale_mark_rate_limit_and_gross_limit` | M1A 基础/边界 | `tests/core_tests.cpp:131` | PASS / PASS / PASS |

## 外部与后续能力

J/K/Q 契约、真实 CTP/SimNow、发送、日志文件、恢复资格和生产回报队列不在这张通过清单中。
测试请求它们时只验证返回 NotImplemented/Blocked，不能据此宣称相应模块实现通过。
