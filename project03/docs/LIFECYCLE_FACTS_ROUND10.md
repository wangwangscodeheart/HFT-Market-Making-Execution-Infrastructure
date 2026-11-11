# 第十小轮：授权许可与查询覆盖值契约

日期：2026-11-06。用户已授权按目标自主推进，不再逐阶段等待审核；依赖与工程检查仍保留。

## 新增内容

`dispatch_facts.hpp` 定义独立的授权 ID、许可 ID、授权事实、许可状态和停止证据。授权引用 command/order/reservation、run、业务作用域、dispatchEpoch、requiredJournalPrefix 和有效期；检查身份完整性及实例一致性。

许可状态明确分为 Queued、Claimed、CallCommitted、CallReturned、AbortedBeforeCall。KillAccepted、NewOrderCallsQuiesced、ExternalRiskResolved 为独立可未知事实，默认不把未知写成已完成。

这些可复制对象不是安全令牌，不提供 claim/TryCommitCall/SDK 入口，也不检验实际库存、持久化或当前门闩。结构正确不代表有权发送。

`recovery_facts.hpp` 定义强类型恢复/dispatch/丢失代次及 latchVersion；查询身份包含 run、账户、queryId、recoveryEpoch、lossGeneration 与源会话。查询描述包含实际合约集合或明确的全账户范围、交易日起止、订单/成交/持仓/资金种类、活动/适用终态范围、逐笔身份范围、profile、边界方法、分页、结束状态、缺项和原始证据引用。

形状检查拒绝错乱日期、重复合约、模糊范围、非法枚举、分页矛盾和超限元数据；相关性检查拒绝错运行、账户、查询、源会话或代次。

Unknown 边界、仅活动订单范围、正常 QueryEnd 与缺项可以共存并被准确记录。形状正确不等于恢复范围足够，也不把空查询解释成订单不存在。查询未执行、证书未签发、gate 未复位。

本地值契约容量边界：合约项和缺项/材料引用各最多 256，页计数最多 4096，标识及单个引用最多 128 字节；这是实现资源边界，不是交易所限制。调用者需处理分配异常，未宣称热路径零分配。

## 实际验证

新增八项结构/边界与关联回归。MSVC C++20 Debug、Release 各 120/120、25400 次断言、0 失败。两类演示 14 个快照与文本 Demo 状态摘要匹配；工作台数据绑定与交互逻辑检查通过。未新增原生浏览器截图验证。

证据目录：evidence-local/2026-11-06-lifecycle-types-debug 与 evidence-local/2026-11-06-lifecycle-types-release。

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\project03\scripts\build-msvc.ps1 -RunName 2026-11-06-lifecycle-types-debug -Configuration Debug
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\project03\scripts\build-msvc.ps1 -RunName 2026-11-06-lifecycle-types-release -Configuration Release
node .\project03\scripts\prepare-cases.cjs 2026-11-06-lifecycle-types-release
node .\project03\scripts\check-workbench.cjs
```

## 后续

本轮补充 P0-02/P0-05 在 M1A 所要求的主要值契约；真实状态机/覆盖资格仍留给对应运行时。继续完善订单内部/客户端/厂商关联语义、受控命令 DTO 与当期验收映射，再依据工程验证推进后续阶段，不等待例行人工审批。
