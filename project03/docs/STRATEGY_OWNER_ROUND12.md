# 第十二小轮：策略身份与订单归属

日期：2026-11-06。M1A 的身份与所有权细化，无自动报价算法、LocalSim 对端或多策略交易运行时。

## 身份分层

StrategyId 是明确的策略归属，不从客户端 producer、行情源或回报通道推断。客户端 producer 可以代表 CLI 或测试输入来源；它与业务策略是两个概念。

当前 CoreConfig 配置一个 StrategyId，M1A 夹具预设为 fixture.default.strategy。SubmitOrder 可显式声明该策略；省略时继承当前单策略配置，而非猜测来源。空标识或超 128 字节拒绝；不同策略的请求在分配业务序号/建单前返回 WrongIdentity。

每个 Order 保存策略身份，客户端索引使用 ClientRequestKey(strategy, producer, requestValue)。该键为后续扩展预留维度，但当前单核心仍只接受一个配置策略。不同策略测试验证键不合并，不证明可将同一账户风控拆到两个交易核心。

## 核心与风险边界

策略与订单、请求索引的双向归属参与 CoreLoop 不变量；提交失败仍整体回滚。成交通过已有内部订单关联获得策略，不从交易通道名称重新分配。

PositionLedger 和 BusinessScope 继续描述账户/合约/交易日基线，不加入策略维度。账户四桶与最坏风险区间不因策略标签重复或切分；将来多策略需显式的库存归属和层级限额，不能把多个策略净仓相抵当成风控完成。

DispatchAuthorizationFact、NewOrderCommandFacts 加入策略身份并检查一致性；仍只为值描述，不产生发送资格。诊断摘要提升 m1a.v5，记录配置、订单与客户端索引的策略字段，历史日志不修改。

## 验证

新增八项：非法策略配置拒绝；省略策略绑定配置而非 producer；外来策略不能命中已有请求；同号客户端键按策略区分；提交失败回滚；账户持仓与风险不拆分；回报来源不改变归属；命令与授权描述策略一致。

MSVC C++20 Debug、Release 各 140/140，25645 次断言、0 失败。两类演示 14 个快照与文本 Demo 摘要匹配，工作台展示实际导出的策略 ID。新日志：

- evidence-local/2026-11-06-strategy-owner-debug/
- evidence-local/2026-11-06-strategy-owner-release/

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\project03\scripts\build-msvc.ps1 -RunName 2026-11-06-strategy-owner-debug -Configuration Debug
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\project03\scripts\build-msvc.ps1 -RunName 2026-11-06-strategy-owner-release -Configuration Release
node .\project03\scripts\prepare-cases.cjs 2026-11-06-strategy-owner-release
node .\project03\scripts\check-workbench.cjs
```

未新增原生浏览器截图验证；没有策略收益、自动下单、真实行情或性能结论。

下一步更新当期 M1A 验收映射与可复现证据，核对剩余阶段适用项和目标工具链；不再逐阶段等待人工审核。
