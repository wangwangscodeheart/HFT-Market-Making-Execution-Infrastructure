# M1B 本地证据门槛复核

日期：2026-11-10。范围为固定本地 profile、单账户/策略/合约、同步纯内存运行。依据交接架构 §17.2，门槛为 A02、A03、完整报价轨迹及独立本地查询证据；线程和耐久日志另属 M1C/M1D。

| 要求 | 实现及可定位的测试/产物 |
|---|---|
| A02 缺档、无效值 | `market_state_tests.cpp` 中 A02_MARKET_missing_fields_or_required_depth_invalidates_previous_quote、A02_MARKET_rejects_invalid_ticks_crosses_quantities_and_depth_order；新场景第七帧实际撤单 |
| A02 过期与旧快照 | A02_MARKET_expiry_boundary_and_duplicate_cannot_refresh_receive_age、A02_MARKET_reconnect_needs_new_generation_and_fresh_snapshot；新场景第三至六帧及第九帧 |
| A02 来源、代次、年龄 | MarketState 分开检查 source_epoch、subscription_generation、receive_sequence 和 received_at；新场景展示原始输入及逐帧 FeedHealth |
| A03 相同输入确定性 | verify_local_quote_trace 从同配置新建运行器逐帧比较策略、核心调用、订单、对端和健康；九种固定场景均走此核验 |
| A03 单侧库存限制 | STRATEGY_inventory_limit_suppresses_risk_increasing_side_and_routes_reduction；七帧 inventory 场景验证九手到十手、减仓及恢复目标 |
| A03 撤旧等待 | PLANNER_changed_quote_cancels_before_creating_and_pending_cancel_keeps_slot；报价场景比较实际取消证据在新单之前 |
| A03 自身订单相交 | PLANNER_stale_generation_foreign_scope_crossed_quotes_and_kill_fail_closed；RISK_crossing_pending_cancel_and_unknown_orders_still_blocked；计划和核心分别阻断 |
| 完整报价与撤成 | 原有 cycle、expiry、query-gap、retry、overflow、cleanup、late-fill、inventory 加新 feed-recovery；导出实际调用、回报和 B/H/P/L/V |
| 独立本地查询 | LocalSim 独立维护订单、成交、持仓、冻结和预算；完整查询校验后与核心比较；新场景第十一帧 query_aligned=true |

“完整”只指这些固定实验及给定输入序列，不是全市场输入采集、真实事故全量复刻、耐久审计或全业务覆盖。回放不证明输入原始真实性。固定数据的行情 origin 是 InterfaceFixture，成交是 LocalSimulator。

结论：上述限定范围的 M1B 本地工程证据门槛通过。2026-11-10-feed-preview-debug / release 均通过 278 项测试、28,304 次断言；十四份功能输出一致，新场景查询对齐与回放检查通过，Release 当前源码和 40 份产物哈希核验通过。这是本地证据结论，不代替用户验收，也不是生产就绪结论。

后续运行时遵循已批准的有界 mutex / condition_variable 队列路线；本报告不把后续阶段功能提前记为已实现。旧顶档夹具与新完整快照模式仍并存，不据此声称所有数据入口都来自真实协议。
