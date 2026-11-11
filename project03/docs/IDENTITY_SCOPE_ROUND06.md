# 第六小轮：业务作用域与身份隔离

日期：2026-11-05。专项核对 M1A 的身份/归属边界，未进行全部架构契约的最终验收。

## 实现

新增 `InstrumentKey`（交易所、实际合约）和 `BusinessScope`（账户、合约键、交易日、持仓基线代次）。品种保持为合约元数据，避免给同一经济合约引入另一个会分裂账本的身份维度。

CoreLoop 持有不可变 scope，通过只读接口导出；委托账户检查、成交交易所/合约检查与逐步不变量使用该作用域。每笔订单和正常/异常成交必须属于当前账户、合约及交易日，订单还必须属于当前实例。没有引入多合约容器、路由器或共享账户风控。

不修改成交经济去重键：仍为账户、交易日、交易所、原始成交作用域和原始成交 ID；Query/Push 不进入经济键。交易所/合约代码不擅自转大小写或拼接月份。

原先空状态摘要没有编码账户，导致不同账户在未下单时可能得到相同诊断摘要。新增回归并将诊断摘要版本提升到 `m1a.v2`，加入完整 scope。此格式不是 Journal、耐久快照、密码学哈希或兼容性恢复协议；历史 v1 日志保留不改写。

## 新增七个测试

| 测试 | 检查意义 | 对应边界 |
|---|---|---|
| 交易所限定合约键 | 同名合约与不同交易所不合并，不猜测大小写 | A01，身份基础 |
| 独立核心作用域 | 两实例使用同号订单序号/成交 ID 仍各自记账 | A01/A08，非共享账户组合风控 |
| 外来成交隔离 | 错账户/交易所/合约/日期/实例不记仓、不释放资源，保留隔离证据 | A08/O06 |
| 外来委托拒绝 | 错账户/合约/日期不建立订单，不消费订单 ID | A01/A04 |
| 外部持仓观察 | 外来范围拒绝；合法未来日观察保存但不覆盖业务桶 | P02/P03 |
| 启动范围限制 | 合约/锚点不一致或非 SHFE/RB 配置拒绝启动 | A01，首版范围 |
| 空状态摘要区分 | 空仓/无订单时不同账户也具有不同摘要 | 诊断/确定性比对基础 |

## 实际验证

MSVC C++20，Debug 与 Release 各 93/93，25120 次断言，0 失败；/W4 /WX。两种构建的正常/竞态 snapshot.json 字节完全一致，每种构建的 14 步快照均与文本 Demo 状态摘要相同。工作台节点绑定、案例切换、持仓表、趋势、来源对照和播放结束检查通过；未做新的原生浏览器截图检查。

原始日志：

- `evidence-local/2026-11-05-scope-final-debug/`
- `evidence-local/2026-11-05-scope-final-release/`

此前 92 例中间构建记录保留于 identity-scope / identity-release 目录；最终以上两个目录为准。

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\project03\scripts\build-msvc.ps1 -RunName 2026-11-05-scope-final-debug -Configuration Debug
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\project03\scripts\build-msvc.ps1 -RunName 2026-11-05-scope-final-release -Configuration Release
node .\project03\scripts\prepare-cases.cjs 2026-11-05-scope-final-release
node .\project03\scripts\check-workbench.cjs
```

## 仍保留的边界

每个核心仍为单账户、单实际 RB 合约。隔离测试不证明支持并行多合约、跨品种组合保证金、跨核心共享账户限额或 CTP。真实参数与外部映射仍待核验。没有新增运行时、Journal、网关或策略。

这轮不等于所有 M1A 类型已齐全：ClientOrderId/策略身份、发送与恢复相关 DTO 的阶段适用性，以及核心序号分配与来源核验顺序，仍需单独对照 V1.1 审查，不能用本专项测试通过代替整体验收。
