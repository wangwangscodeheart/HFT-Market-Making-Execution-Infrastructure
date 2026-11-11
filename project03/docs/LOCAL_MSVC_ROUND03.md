# 第三小轮：Windows C++20 候选复现

日期：2026-11-05（Asia/Tokyo）。本轮仅处理 M1A 候选的 MSVC 移植与本机复现，不代表正式验收。

## 工作副本

`project03/` 来自交接包的 M1A 候选，保留 include、src、tests、examples、docs、scripts 与 CMake 配置。原始交接目录、ZIP 和历史证据未修改。新的运行记录位于 `evidence-local/2026-11-05-msvc/`。

## 原因与修正

MSVC 的 map 移动构造不是 noexcept；包含 map 的 ExecutionLedger、FundsLedger、CoreState 默认交换依赖移动构造，原 CoreState 的静态断言因而失败。

为三个类提供 ADL swap，使用容器自身交换或逐成员交换；内部静态断言继续验证交换不抛异常。core.cpp 原来的 `is_nothrow_swappable_v<CoreState>` 断言保留，业务流程不变。

CoreLoop 原先通过 copy → apply → validate → swap 提交状态：先在候选副本上计算，只有通过不变量检查才交换至正式状态。无异常交换保证最后一步不会提交到一半。以后新增成员时必须同时更新 swap，并继续运行回滚与业务测试。

CMake 按编译器选择严格警告选项：MSVC /W4 /WX /permissive- /utf-8 /EHsc；其他编译器沿用原设置。未关闭警告、删除测试或改变预期。

## 实际执行

在工作区根目录：

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\project03\scripts\build-msvc.ps1 -TraitsOnly
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\project03\scripts\build-msvc.ps1
```

ExecutionPolicy 参数只对本次 PowerShell 进程有效，不修改机器或用户策略。脚本在 VS x64 开发环境中运行 CMake/NMake、CTest、聚合测试和示例；任何失败立即停止，避免运行旧二进制。

结果：Debug 构建成功；CTest 85/85；聚合 `RESULT cases=85 failed=0 assertions=24868`。Demo 退出码 0，最终 `Q=3 B=2 H=0 P=0 L=1 V=0 booked_long_today=2`。

候选 Demo 注入参考价、委托、成交 t1、撤单请求、累计成交 2 的撤单终态、迟到 t2、重复 t2 和不可用的发送请求。它没有柜台连接、LocalSim 对端或实际外发。

## 限制与下一小步

本轮仅复现 MSVC Debug，未重跑 GCC、Release、Sanitizer、性能和真实接口测试。前端仍用独立教学数据，尚未读取本轮真实输出。

下一小步可导出实际 C++ 逐步结果供离线工作台读取；不引入 WebBridge、网络控制、线程或外部交易接口。多品种/多合约身份与配置基础另做小范围设计和验证，不在移植修正中扩大业务范围。
