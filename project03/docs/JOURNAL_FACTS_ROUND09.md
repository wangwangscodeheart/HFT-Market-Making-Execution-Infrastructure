# 第九小轮：Journal 身份与前缀值契约

日期：2026-11-06。按 V1.1 的 M1A 类型要求补齐部分 P0-01 数据契约，未实现持久化或发送运行时。

## 变更

`include/p03/journal_facts.hpp` 提供 JournalId、WriterEpoch、JournalSequence、JournalBatchNumber、JournalIdentity、JournalBatchBoundary、RequiredJournalPrefix、LiveJournalPrefixes 和 JournalDurabilityAcknowledgement。

不同计数器类型不可相互隐式转换；JournalId 的本地值形状检查不规定磁盘格式或真实身份分配方式。

实现的检查仅限形状和显式给定数据之间的关系：

- 批次必须包含非零起始序号、合法末端及正确前驱；空洞、重叠、零身份和计数回绕拒绝。
- 相邻批次必须同 journal/writer，batchNo 连续且前驱末端准确相接。
- 活动前缀必须满足 acknowledged ≤ durable ≤ written ≤ produced，空日志允许全零。
- 确认引用必须同 journal/writer、同 batchNo，并指向指定批次末端；落在批次中间或越过末端均无效。

这些检查不证明所给边界已在真实索引中登记，不证明 fsync 成功，不允许 claim 或下单。也不提供恢复 recoveredSeq 推导；恢复前缀不能被当成旧发送许可。所有发送/Journal 能力仍保持未实现。

## 通俗说明

“生成了日志”“写调用接收了字节”“磁盘同步成功”“核心收到确认”是四件不同的事。未来发送新订单，需要关联同一个 journal 和同一代 Writer 的有效前缀，不能只取看到的最大数字。本轮先把这些字段和最基本的关系分开，避免后续接口把它们混成一个整数。

## 实际验证

新增六项类型/边界回归。MSVC C++20 Debug、Release 各 112/112，25346 次断言，0 失败。正常与竞态两案共 14 个快照仍与各自文本 Demo 状态摘要完全一致。新日志在：

- evidence-local/2026-11-06-journal-types-debug/
- evidence-local/2026-11-06-journal-types-release/

本轮没有新的原生浏览器截图验证。

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\project03\scripts\build-msvc.ps1 -RunName 2026-11-06-journal-types-debug -Configuration Debug
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\project03\scripts\build-msvc.ps1 -RunName 2026-11-06-journal-types-release -Configuration Release
node .\project03\scripts\prepare-cases.cjs 2026-11-06-journal-types-release
node .\project03\scripts\check-workbench.cjs
```

## 核对结果与下一步

见 M1A_CURRENT_GAPS.md：许可/授权事实、丢失代次与恢复查询覆盖类型、客户端订单关联语义等仍未关闭。未将本轮类型检查当成 J01～J07 的真实文件实现验收，也未把 M1A 标记 Accepted。
