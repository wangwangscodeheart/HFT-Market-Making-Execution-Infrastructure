# M1A 架构对应、阶段边界与实施取舍

基线原文见 `architecture/V1.1-approved.md`。该原文是审查时的不可改历史文本，其中“待最终审查”字样由用户后续正式批准覆盖：V1.1、ADR001～ADR010 已批准，当前实施授权只到 M1A。没有改变市场、接口主线或 ADR008。

## 当前类与 V1.1 的对应

| V1.1 章节/ADR | 实际类/行为 | 本阶段限制 |
|---|---|---|
| §3、§5.1、§6；ADR001 | CoreLoop 私有 CoreState、同步 step；StepHarness 按逻辑时间/插入序排序 | 单写者调用约定；无线程运行时、互斥队列、并发安全保证 |
| §4、ADR002/010 | 标准业务类型与 fixture 证据来源；ExternalUnconfigured 明确 Blocked | 统一业务类型已建立；完整网关命令端口、CTP/LocalSim 实现尚未建立 |
| §6、§7.4 | 四类强身份、实例/单调序号、独立外部身份类型 | 没有持久化 ID 分配、CTP OrderRef/OrderSysID 绑定 |
| §7、ADR005 | Order 的四维状态、R 下界/F 证明、B/H/P/L/V、独立撤单历史 | 发送维度为 NotImplemented，不假装处于 HandedOff；PendingNew/Pending 留待真实发送阶段 |
| §7.3～7.5、P0-03 | ExecutionLedger 的经济键、B 与 X；查询/推送共用归并 | fixture 键范围只证明本地规则；真实 TradeID 唯一性待核验 |
| §8.1、ADR004 | copy/validate/noexcept swap 原子业务提交，预占随分区重算 | 动态分配、O(状态大小) 复制；不宣称热路径零分配或低延迟达标 |
| §8.2 | RiskEngine 的净仓区间、总仓、可平量、数量/资金/自成交检查 | 无策略自动报价；ReferenceMark 为手工输入，非实时行情接口 |
| §8.3 | 逻辑时间行情过期、意图准入速率限制 | 未实现 Adapter/SDK/撤单发送层节流 |
| §9、P0-04 | 固定 PositionAnchor、四桶、冻结、独立外部/结算观察、FundsLedger | 启动种子+增量；跨日只阻断；真实持仓映射、换锚与结算核算 Blocked/未实现 |
| §16.2 | 显式容量、小容量失败用例、去重身份不自动回收 | 不是运行时队列、回报溢出证书或资源耗尽实验室 |
| §17.2 | 身份、合约/日历、四桶、OMS、证据、预占、StepHarness | 交付 M1A；未进入 M1B～M1F |

## 精确的未实现/阻塞状态

| 能力 | 程序返回 | 所属后续工作 |
|---|---|---|
| 订单发送 | NotImplemented | M1B 业务对端，M1C 调度，M1D 持久化发送 |
| EmergencyCancel | NotImplemented | 按 ADR008 实现受限减险、AuditIncomplete 与强制外部对账；M1A 不外发 |
| JournalAppend、连续提交前缀、文件失败 | NotImplemented | M1D，J01～J07 未验收 |
| RestoreSnapshot、持久化回放 | NotImplemented | M1D 基础及后续恢复；当前摘要不是快照 |
| LocalSimulator、报价策略 | NotImplemented | M1B；当前演示仅注入业务事实 |
| ThreadRuntime、队列、DispatchGate/claim/SDK 竞态 | NotImplemented | M1C，K01～K07 未验收 |
| WebBridge、Web 工作台 | NotImplemented | M1E |
| Reactivate/自动解除停止 | NotImplemented | 需要覆盖证据与恢复门禁，禁止用开关清除故障 |
| CtpConnect | Blocked | 精确 SDK、认证、连接条件和接口语义待确认；当前无 SDK 依赖 |
| ReconcileExternal、真实查询覆盖、真实持仓映射 | Blocked | 无外部能力证明；Q01～Q09 未验收 |
| SMDP 编解码/恢复、事故实验室、性能测量 | 未建立实现入口 | 独立实验/后续阶段，不能借本次测试宣称完成 |

## 有意采用的实现取舍

1. **先采用一个 p03_core 库目标。** 逻辑类按头文件/实现分开，目录先收敛为 include/src/tests。没有预建 adapters、web 等空目录或成功桩；对应架构中的逻辑模块，不改变依赖方向。
2. **事务复制而非复杂回滚代码。** 候选 CoreState 完成 OMS、冻结、资源更新及不变量检查后 noexcept swap；拒绝或预提交失败丢弃候选。代价是复制和分配；未来有测量依据再优化，必须保留同一套业务测试。
3. **小型标准库测试运行器 + CTest。** 不依赖在线下载 GoogleTest；检查不使用 `assert`，Release 下继续有效。支持单例、清单、失败退出码，CTest 提供每例隔离、超时与 JUnit。
4. **测试来源明确。** 当前 LocalFixture 与未来 LocalSim/CTP 是不同概念；fixture 仅代表允许测试注入的证据。创建显式非 fixture 合约会在启动时失败，外部来源不会自动退回 fixture。
5. **资金采用保守的持仓占用保留。** 平仓后暂不返还曾经记账的开仓模型保证金。这样不虚构外部退款；会降低连续运行可用预算，后续必须以已核验换锚能力解决，不能用于真实资金准确性宣称。
6. **没有生产恢复动作。** BoundaryHalted、AwaitDayEvidence、ConflictHalted 都没有 M1A 清除接口。符合缺证据保持停止的规则；不意味着已经实现恢复门禁或完整恢复状态机。

以上为 M1A 实施细化和公开限制，不申请改变任何已批准 ADR，不以本阶段代码替代 M1C/M1D/M1F 的验收。
