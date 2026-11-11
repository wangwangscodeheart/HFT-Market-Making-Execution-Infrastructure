# 实际实施状态与证据分层

**目前没有通过 M1A 正式验收的完整交易系统实现。** 但不能写“当前没有业务代码”：上一轮已有 M1A 候选源码、85 项测试和交付报告。本包仅迁移、静态核对和保留这些证据，不重新编码，不替用户验收。

## 状态标签

DESIGN_APPROVED＝设计获准；MODEL_TESTED＝独立模型有原始运行记录；CANDIDATE_IMPLEMENTED＝有实际业务类源码、待审；HISTORICALLY_TESTED＝历史日志可核对、此次未重跑；ACCEPTED＝必须有用户正式验收消息；BLOCKED＝缺外部材料/权限；NOT_STARTED＝尚未有实施证据。不能把这些标签合并成“全部完成”。

| 项目 | 真实状态 | 证据与局限 |
|---|---|---|
| M0 市场与主接口 | 完成研究并获批准 | M0 原件、用户 C02；当前 SDK 精确包未冻结 |
| V1.1 与 ADR001～010、五 P0 | DESIGN_APPROVED | 原件、变更清单、C04；原件待审文字是历史 |
| C++20 参考模型 | MODEL_TESTED | 原 ZIP 含 CPP、README、38 PASS 输出、执行元数据；无真实线程/fsync/SDK |
| M1A 工程与业务类 | CANDIDATE_IMPLEMENTED | `implementation/m1a_candidate/project03/`；源 ZIP 保持不变 |
| M1A 测试 | HISTORICALLY_TESTED，未正式验收 | Debug/Release/ASan+UBSan 各 85/85、24,868 检查；干净 Release 85/85；LSan 未验证 |
| M1A 用户验收 | **待审查** | 没有用户正式批准 M1A 通过的消息 |
| M1B 策略/LocalSim 闭环 | NOT_STARTED，未授权 | 手工事实输入演示不是模拟柜台或策略 |
| M1C 运行时/队列/gate | NOT_STARTED，未授权 | CoreLoop 是同步单写者业务调用；不是四线程实现 |
| M1D Journal/持久化发送/恢复 | NOT_STARTED，未授权 | 业务摘要不是快照；没有耐久性/重启恢复保证 |
| M1E WebBridge/页面 | NOT_STARTED，未授权 | 七页设计保留，代码没有 Web |
| M1F 阶段复核/闭环恢复 | NOT_STARTED，未授权 | 干净 M1A 构建不等于完成 M1F |
| CTP/SimNow | BLOCKED/未连接 | SDK、ABI、认证、前置、权限、能力均需核验 |
| SMDP 独立协议实验 | DESIGN_APPROVED，未实现 | 官方目录/链接；夹具、编解码和恢复均未实施 |
| 四个历史事故实验 | 事实与方案已整理，未运行 | 不能把 38 模型检查或 M1A 失败测试记为事故实验结果 |
| 性能/压力/长稳 | 无项目测量证据 | CTest 用时不是 P99.9；无实盘收益或生产部署证据 |

## 候选实现的源码定位

| 文件（相对候选 project03） | 可由源码支持的能力 | 不包含的能力 |
|---|---|---|
| `types.*` | 强 ID、整数数量/价格、交易日、来源和错误类型 | 持久化 ID 分配/SDK 字段绑定 |
| `reference_data.*` | 配置合约、tick/价格带/手数、显式会话窗口 | 真实合约目录下载、生产日历加载 |
| `positions.*` | 四桶、固定启动 anchor、冻结及独立 fixture 观察类型 | 真实 CTP 聚合持仓映射、结算核算、换锚 |
| `oms.*`、`ledger.*` | R/F 证据、B/H/P/L/V、X、共用去重、撤单历史 | 真正发送/SDK/外部查询覆盖 |
| `risk.*`、`funds.*` | 原子预占目标、最坏区间、总仓、预算及 Li/Ci 唯一证据 | 精确柜台保证金试算、自动退款/权益计算 |
| `core.*` | 唯一写入口、copy/validate/swap、停止和跨日阻断、能力错误 | 线程运行时、文件 Writer、实际 gate/claim |
| `step_harness.*` | 逻辑时间/插入序调度、输入/结果/摘要 | 物理时延基准或持久化 ReplayRunner |

`SendState` 固定 NotImplemented。FutureOperation 中 CtpConnect/ReconcileExternal 为 Blocked，其余发送、EmergencyCancel、JournalAppend、LocalSimulator、WebBridge、ThreadRuntime、RestoreSnapshot、Reactivate 均 NotImplemented。不能把能力负向测试的 PASS 当作这些功能实现通过。

## 已公开的取舍与待审差异

候选只有 `p03_core` 库及测试/演示，按 include/src 收敛目录；使用动态分配与 O(状态大小) 候选复制，未宣称低延迟达标。保证金对已记账开仓保守保留，平仓不自行返还外部可用；只支持固定启动种子+增量，跨日只阻断。参考模型未复制为业务实现。

V1.1 在各 P0 的 M1A 影响中还提到 journal/writer 身份、requiredJournalSeq、许可事实、lossGeneration 和覆盖类型。当前候选头文件未见这些完整发送/恢复 DTO；这是接管验收应逐项核对的范围问题，不能由本包擅自裁决“全部已满足”，也不能借补类型开始写运行时或文件实现。发送维度缺少 PendingNew/Pending 是已公开的阶段限制，需按 M1A 适用范围审查。

原候选验收报告及 85 测试清单保留于源码包 docs；它们是实施方提交，不是用户验收结论。先复现与审查已有候选，避免重新实施后丢掉原始工作和失败历史。
