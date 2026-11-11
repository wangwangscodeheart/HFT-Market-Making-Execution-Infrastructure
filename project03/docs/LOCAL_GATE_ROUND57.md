# 第五十七轮：同锁发送门闩与关键收件箱

日期：2026-11-10。范围是 LocalDispatchGate 内存实验组件，不是生产网关或完整 RuntimeRunner。

门闩绑定 instance、run、业务范围、策略、dispatchEpoch 和显式内存前缀。许可注册有固定上限，保留已结束许可且不复用槽位；相同 command 或 authorization 拒绝重复注册。claim 检查门闩、代次、到期与内存前缀；TryCommitCall 再检查一次资格。状态迁移、关闭及观察使用同一互斥锁，外部调用在锁外。

Kill 关闭新增领取和调用承诺，提升 epoch 与 latchVersion。已 Claimed 的许可在随后承诺时变为 AbortedBeforeCall；已经 CallCommitted 的许可仍在途，直到明确 call_returned。重复承诺、返回、已承诺后撤回均不能产生第二次调用资格。计数器耗尽保持关闭，不回绕。门闩不直接操作仓位、风险预占或订单终态。

LocalCriticalInbox 将有界队列与共享门闩组合。关键消息入队失败时，在返回生产者之前同步关闭门闩并增加 lossGeneration。消费者排空队列不会清除此门闩；后续消息入队成功也不会恢复发送资格。这不是“检测到满队列瞬间就没有其他并行调用”：真正边界是门闩同锁关闭点，关闭之前已承诺的调用仍可进入外部。

测试枚举 K C B I R、C K B I R、C B K I R、C B I K R、C B I R K 五种 Kill 位置；另以 barrier 和真实线程控制 claim/Kill/commit 与 commit/Kill/入口/返回的交错。关键收件箱溢出同时覆盖 Claimed 与 CallCommitted 两种状态，并验证排空后不重开。还验证身份、前缀不足、到期、重复、容量与计数器耗尽。

静止表示门闩关闭且 Claimed、CallCommitted 均为零；Queued 项仍须交接解释，静止不代表所有队列为空，更不代表 ExternalRiskResolved。外部调用“返回”没有被解释为 DefinitelyNotSent。组件不提供复位，尚未实现恢复证书、旧队列覆盖检查、节流和核心心跳推进。

显式 memory_prefix 只是固定本地实验的资格标记，绝非 Writer 确认或 fsync；没有接入实际 SDK、日志文件、核心命令输出或完整四上下文运行器。公开的事实 ID 也不是跨信任边界的不可伪造能力。后续适配器必须独占许可使用，绑定核心实际发出的授权及日志资格，不能让页面或任意调用方直接操作此实验门闩。

最终验证：2026-11-10-local-gate-debug / release 干净构建均通过 292 项测试、28,446 次断言，零失败。新增七项门闩测试经 CTest 和聚合执行通过；证据报告、页面逻辑及离线包检查通过，Release 当前源码和 40 份产物哈希核验通过。十四份功能输出在 Debug / Release 间、与第五十六轮之间均逐字节一致。未验证 Linux、TSan 或长稳；没有据此宣称 M1C 完成或实盘安全保证。
