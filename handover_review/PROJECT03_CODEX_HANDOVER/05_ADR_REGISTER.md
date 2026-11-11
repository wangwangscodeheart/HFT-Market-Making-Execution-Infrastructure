# ADR001～ADR010 登记册

当前十项设计决策均随 V1.1 正式获批；ADR008 按先前明确批准的受限 EmergencyCancel 契约。原件§18的“建议/有条件通过/待审”是提交时状态，用户 C04 覆盖批准状态，不改变技术内容。

历史资料只有架构正文与登记表，未找到十份独立ADR文件原件。本次 `adr/ADR001.md`～`ADR010.md` 是可执行的交接整理，不冒充历史原件。每项包含背景、决定、原因、约束、状态、代价和对应章节；详细P0全文在06。

| ADR | 名称 | 当前设计状态 | V1.1位置 | 实施边界 |
|---|---|---|---|---|
| [ADR001](adr/ADR001.md) | 单核心写者、单账户单合约先行 | Approved | §1.1、§2.3、§3、§5.1、§6、§18 | M1A候选具同步单写者与const视图；四线程运行时未实现。 |
| [ADR002](adr/ADR002.md) | 标准业务端口与独立 AdapterProfile | Approved | §4、§11、§18 | M1A只有业务类型/fixture来源和Blocked边界；完整网关端口、LocalSim与CTP实现未完成。 |
| [ADR003](adr/ADR003.md) | 初版有界互斥队列 | Approved | §5.2、§16.2、§18 | 未实现真实运行队列；StepHarness的有界调度列表不是该ADR的并发实现。 |
| [ADR004](adr/ADR004.md) | 预占与 OMS 原子业务更新 | Approved | §3、§8.1～8.3、§9、§17.1/A04、§18 | M1A候选使用copy/validate/noexcept swap；O(状态大小)复制和动态分配未测量，不是新增性能决策。 |
| [ADR005](adr/ADR005.md) | 四维 OMS 与成交悬置 | Approved | §7、§8.1、§9、§18 | M1A候选已有业务实现/85测试记录，发送维度固定NotImplemented；未正式验收。 |
| [ADR006](adr/ADR006.md) | 新单在意图持久化后释放 | Approved | §8.3、§12.1～12.4、§18 | 模型J族通过；真实Writer/文件/持久化发送未实现，M1A无发送资格。 |
| [ADR007](adr/ADR007.md) | Kill 区分停止领取与发送静止 | Approved | §5.3～5.4、§8.4、§18 | 5个插入位置只在模型；真实gate/mutex/FakeSdk/barrier/TSan未实现。 |
| [ADR008](adr/ADR008.md) | 日志不可写时受限 EmergencyCancel | Approved | §8.4、§12.1、§12.6/Q08、§18 | 设计先行正式批准；模型Q08有检查；M1A请求EmergencyCancel返回NotImplemented。 |
| [ADR009](adr/ADR009.md) | WebBridge 独立进程与只读投影 | Approved | §5.1、§14、§18 | 仅设计，WebBridge/BridgeIO/页面尚未实现。 |
| [ADR010](adr/ADR010.md) | 本地模拟不证明真实 CTP 能力 | Approved | §2.1、§4.3、§10、§11.2、§18.1 | M1A只有LocalFixture；不是LocalSim，更不是CtpProfile完成或SimNow成功。 |

M0§15曾建议一组八个讨论主题，没有冻结为当前编号；不得把该列表代替本登记。初始备选在02和原M0/V1.0保留，五P0演化在变更清单。未记录的历史争论不补造。

变更规则：在原ADR上悄悄改意图、把局部实现取舍称为新批准决定、以测试通过自动修改安全契约均禁止。需变更先报告影响和证据，重大方向由用户确认；在已批准范围内实施细节可按源码/测试解释。
