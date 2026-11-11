# 第十一小轮：客户端订单身份、请求幂等与命令描述

日期：2026-11-06。自主推进 M1A 的本地身份关联与事务，不接入柜台或实现发送运行时。

## 身份含义

ClientOrderId 由 producer 命名空间和原始请求值组成，是本地请求标识；不等同内部 OrderId、CommandId、撤单 ActionId、VendorRequestIdentity、CTP OrderRef 或 OrderSysID。每个字符串最多 128 字节，按原值比较，不做大小写或字段编码推断。

当前 SubmitOrder 可选携带 client_id，接受后的 Order 保存关联，CoreState 维护有界索引；演示案例均显式提供本地 fixture 标识。未携带 client_id 的既有调用继续被视为独立请求，没有自动幂等保证，也不会用内部 OrderId 补造客户端标识。

索引由当前单账户/单合约/单交易日核心拥有，不持久化、不自动删除；它不保证跨进程、重启或共享账户多核心幂等。扩展时需要明确将关联登记放到相应账户/运行作用域，而非把多个孤立索引当作全账户唯一性保证。

## 实际核心行为

| 输入情况 | 结果 | 状态与资源 |
|---|---|---|
| 新 client_id | 通过风险后创建新订单与关联 | 与订单、冻结、资源同一事务提交 |
| 同 client_id / 同 OrderIntent | Duplicate，并返回原内部订单 ID | 无新增订单/预占/速率用量；不外发 |
| 同 client_id / 改变经济字段 | EvidenceConflict，并返回原订单引用 | 拒绝本地请求；不篡改或把原订单标成外部证据冲突 |
| 原请求风险不足或提交失败 | 不建立索引 | 确认未接受后可重新提交，不留下幽灵关联 |
| 原订单部分/全部成交、Killed 或未知超时 | 相同请求仍返回原订单 | 不重新开仓、不解除停止、不释放未知风险 |

重试比较 OrderIntent，允许会话检查上下文不同；账户/合约/交易日和客户端标识仍先做准入。返回结果含 order 引用不代表新订单成功，调用者必须检查 error / applied。合法重复仍分配有序业务处理编号，但不消耗新的订单/资源 ID。

新 client_orders 索引参与 CoreState 复制、无异常交换和双向关联不变量；提交前失败时与订单和资源一起丢弃。诊断摘要更新为 m1a.v4，并编码客户端关联；历史摘要不改写。

## 命令与厂商请求描述

`order_command_facts.hpp` 提供 NewOrderCommandFacts，检查意图和授权描述的账户、合约、交易日一致，保留 requiredJournalPrefix。VendorRequestIdentity 保存原始请求值、来源会话与 profile，不规定 CTP 整数编码或伪造 OrderRef。

该类型只是可复制的值描述，不是受控可执行命令或发送能力。CoreLoop 不用虚构 Journal 身份生成它，不把结构校验通过当作授权。真正受控的命令构造与执行须接上真实风险/日志/运行时证据后实施。

## 验证

新增十二项回归，覆盖部分/全部成交后的重试、未知超时、Killed、容量满、不同 producer、同键改内容、非法身份、预占失败、提交回滚、命令作用域和厂商引用形状。

MSVC C++20 Debug、Release 各 132/132，25580 次断言，0 失败。两类演示 14 步快照与文本 Demo 摘要匹配；两种构建的 JSON 完全一致。前端增加实际客户端请求标识，交互逻辑检查通过；未新增原生浏览器截图验证。

新证据：evidence-local/2026-11-06-client-final-debug 与 evidence-local/2026-11-06-client-final-release。此前 client-orders-debug/release 中的 130 例中间记录保留。

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\project03\scripts\build-msvc.ps1 -RunName 2026-11-06-client-final-debug -Configuration Debug
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\project03\scripts\build-msvc.ps1 -RunName 2026-11-06-client-final-release -Configuration Release
node .\project03\scripts\prepare-cases.cjs 2026-11-06-client-final-release
node .\project03\scripts\check-workbench.cjs
```
