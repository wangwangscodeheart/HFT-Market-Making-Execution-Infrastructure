# 第八小轮：输入准入、拒绝事实与编号分离

日期：2026-11-06。依据 V1.1 §6.2 的校验顺序细化同步 M1A 内核，不修改 ADR，不实现 Journal、线程、网关或完整事件信封。

## 三种输入结果

| admission | 何时使用 | ingressSequence | coreSeq / command ID | 状态处理 |
|---|---|---|---|---|
| RejectedInput | 非法本地意图、无效观察、时钟回退等 | 原始尝试占一个编号 | 不分配 | 不推进核心业务时钟或账本 |
| BusinessInput | 来源/身份/完整性通过，包括合法重复、风控拒绝 | 原始尝试占一个编号 | 分配 | 按业务规则处理，重复不重复记账 |
| RejectedEvidenceFact | 必须隔离的非法交易/订单证据 | 原始尝试占一个编号 | 分配给本地拒绝事实 | 保留隔离证据、停报，不将原始载荷当成可信成交 |

入口先检查来源、身份与载荷完整性，再决定是拒绝、不可信证据的本地隔离事实，还是合格业务输入，之后才分配 coreSeq。不会因原始来源不可信而丢弃其潜在风险信息。

StepResult 同时给出两种编号和 admission；StepHarness 保留原始输入和完整结果。隔离记录关联原始 ingressSequence 与本地拒绝事实 coreSeq。核心原始输入计数是进程内调用编号，不是交易所、柜台或外部持久化接收序号。

## 实现边界

前置判定只读业务状态；通过后仍在 apply 中保留防御性校验。FundsLedger 的校验拆成 const validate 与 observe，避免为准入检查提前改变资金证据。合法的同键冲突仍保留原成交，标记关联订单，不覆盖原账本。

普通不合格请求不推进 CoreLoop 的 last_time；StepHarness 自身仍按调度的逻辑时间排序，并保留拒绝尝试。这两个时间职责不同。

诊断摘要版本为 m1a.v3；原始 ingress 计数不进入业务摘要，故纯拒绝尝试不改变业务摘要。拒绝事实确实改变隔离/停止状态，因此在业务摘要中有其状态与编号关联。历史 v1/v2 原始日志不改写。该摘要不是持久化快照，也不承诺恢复格式兼容。

## 回归与实际结果

新增八项回归：拒绝请求不消费业务编号/时钟；不可信成交形成拒绝事实且无记账；错误身份不消费订单或命令 ID；风控拒绝仍是有序业务决定；合法重复仍有序但不重复记账；同成交键的订单关联冲突保留双方证据；Harness 保留原始拒绝与独立编号；坏观察/未知能力请求不推进业务时钟。

最终 MSVC C++20 Debug / Release 各 106/106、25312 次断言、0 失败；两类案例共 14 个快照与文本 Demo 摘要逐步相同，两个构建的原始 JSON 字节相同。Release 断言失败负控返回 1；工作台数据绑定、编号显示、案例切换与图表逻辑检查通过。未新增原生浏览器截图检查。

最终证据保存于 evidence-local/2026-11-06-admission-linked-debug 与 evidence-local/2026-11-06-admission-linked-release。先前 2026-11-05-admission-debug 及当日 final-debug/final-release 为中间构建，均保留作历史，不替代最终记录。

## 复现

工作区根目录：

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\project03\scripts\build-msvc.ps1 -RunName 2026-11-06-admission-linked-debug -Configuration Debug
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\project03\scripts\build-msvc.ps1 -RunName 2026-11-06-admission-linked-release -Configuration Release
node .\project03\scripts\prepare-cases.cjs 2026-11-06-admission-linked-release
node .\project03\scripts\check-workbench.cjs
```

前端事件表现在分别展示 CORE SEQ 与 INGRESS，检查器展示 admission；常规两案全部为 BusinessInput，拒绝分类通过实际 C++ 回归验证。没有把页面自动浏览当作高频运行或实时时间测量。

## 未完成范围

完整 runId/environment/sourceEpoch 信封、外部持久化接收编号、耐久日志、异常分配的完整故障矩阵、恢复与外部接口仍未实现。需要在对应阶段继续核对，不能由本轮准入测试替代。
