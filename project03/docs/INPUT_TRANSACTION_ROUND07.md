# 第七小轮：输入完整性与事务重试

日期：2026-11-05。范围：M1A 输入检查与内存事务，保留现有架构、原始材料及历史证据。

## 修复

TradeReport 的 Channel 只定义 Push/Query，但原核心未检查非法枚举值，可能将其成交记账，或因经济键重复而提前返回 Duplicate。现于去重前检查通道合法性：非法通道返回 InvalidInput、保留隔离材料、停报；不增加成交账本或持仓，不释放资源。

Query/Push 仍不进入经济去重键，合法查询/推送继续共同去重。本地夹具通过不证明真实 CTP 输入映射已验证。

## 新增五个回归

测试以已有正向标准案例和撤单竞态场景为基础，未创建无来源的正常市场走势。

- 标准成交注入非法通道，三个非法值均不能记账。
- 已记账成交使用非法通道再次到达，不能绕过输入校验。
- 正常成交在提交前注入失败：无幽灵去重记录、持仓或资金半提交；同成交重试成功。
- 撤单终态在提交前注入失败：资源不提前释放；重试后才按累计证据释放确定未成交量。
- 重复成交没有提交，不消耗“下一次提交失败”的测试触发器；下一笔新成交失败后仍可完整重试。

这仅使用既有同步 fail_next_commit_for_test 接缝，不是完整故障注入台、进程崩溃或持久化事务测试。

## 验证

MSVC C++20 Debug / Release 各 98/98、25225 次断言、0 失败。两类演示共 14 步快照与各自文本 Demo 摘要相同；两个构建的 JSON 完全一致，并与上一轮合法案例 JSON 完全一致。工作台交互逻辑检查通过，未新增浏览器截图验证。

证据目录：

- evidence-local/2026-11-05-input-final-debug
- evidence-local/2026-11-05-input-final-release

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\project03\scripts\build-msvc.ps1 -RunName 2026-11-05-input-final-debug -Configuration Debug
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\project03\scripts\build-msvc.ps1 -RunName 2026-11-05-input-final-release -Configuration Release
node .\project03\scripts\prepare-cases.cjs 2026-11-05-input-final-release
node .\project03\scripts\check-workbench.cjs
```

## 单独保留的待审问题

coreSeq 分配早于来源/身份检查，与 V1.1 的处理顺序有差异。详见 INPUT_SEQUENCE_REVIEW.md，包含现状、风险和建议的最小细化方案。本轮未改序号语义，不将该问题标记为已解决。
