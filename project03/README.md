# PROJECT 03 · M1A

以已批准的《系统架构设计 V1.1》第 17.2 节为基线，交付 **C++20 内存业务内核与确定性业务测试**。当前待 M1A 验收，未推进 M1B～M1F。

主线保持上期所商品期货 / RB / 普通账户简化双边限价报价 / 后续 CTP TraderAPI + MdAPI。**本版本没有任何柜台连接、交易所连接、策略自动报价或本地柜台模拟器。** `rb_FIXTURE`、交易时段、价格带、数量、保证金及费用都是显式测试材料，不是当前可交易合约或真实柜台参数。

## 快速构建

在本目录执行；需要 Linux、支持 C++20 的 GCC/Clang、CMake ≥ 3.20 和构建工具。本次验证为 GCC 13.3.0 / CMake 3.31.6 / Ninja 1.11.1。核心和测试只依赖 C++ 标准库；Python 仅用于批量收集验收证据。

```bash
cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build/debug --parallel 2
ctest --test-dir build/debug --output-on-failure
./build/debug/p03_tests
./build/debug/p03_m1a_demo
```

```bash
cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build/release --parallel 2
ctest --test-dir build/release --output-on-failure

cmake -S . -B build/sanitizers -DCMAKE_BUILD_TYPE=Debug -DP03_SANITIZE=ON
cmake --build build/sanitizers --parallel 2
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ctest --test-dir build/sanitizers --output-on-failure
```

本次环境不允许 LSan 完成进程任务信息扫描，因此上面的命令只验收 ASan/UBSan；在具备 LSan 条件的 Linux 主机上另行使用 `detect_leaks=1`，不能把这里的通过当作泄漏检测通过。

完整复现并收集日志：

```bash
python3 scripts/verify.py --cmake cmake --detect-leaks 0
# 单个业务测试与测试清单
./build/debug/p03_tests --case O03_A07_cancel_exact_and_late_trade_release_only_one
./build/debug/p03_tests --list
```

批量脚本会创建 `evidence/` 日志，任何非预期退出码都会使脚本失败。负向控制命令 `--self-check-failure`、未知 `--case` 故意返回 1、2，证明 Release 下检查仍有效；这两项不是业务失败。

## 目录与目标

| 路径/目标 | 实际职责 |
|---|---|
| `include/p03/types.hpp`、`src/types.cpp` | 强身份、整数数量/价格、交易日、证据来源、错误与能力类型 |
| `reference_data.*` | 显式合约版本和价格/数量约束；显式民用日期→交易日会话窗口 |
| `positions.*` | 四桶、按桶冻结、固定 PositionAnchor、外部观察与结算观察类型 |
| `oms.*` | 执行/发送/撤单/一致性四维状态，累计证据、终态精确量、撤单动作历史 |
| `ledger.*` | 查询/推送共用的经济成交键、正常 B 与异常 X 账本 |
| `funds.*` | Li/Ci 资源覆盖、覆盖证据唯一归属、独立本地/外部预算 |
| `risk.*` | 最坏净仓区间、总仓、平仓可用量、资源目标、自成交检查 |
| `core.*` | `CoreLoop::step` 唯一业务写入口；候选状态事务、不变量、停止与跨日阻断 |
| `step_harness.*` | 单线程逻辑时间排序；保存输入、结果、规范化业务摘要 |
| `tests/` | 对上述真实类的单元和组合测试；不编译或复制 V1.1 的独立参考模型 |
| `examples/m1a_demo.cpp` | 手工事实输入演示 Q=3 的撤成与迟到成交；没有对端模拟 |
| `docs/` | 验收、架构映射、不变量、学习与调试说明 |
| `evidence/` | 构建、测试、JUnit、失败历史、环境和文件指纹 |
| `p03_core` | 一个库目标，包含当前业务类和同步执行器；没有运行时线程/SDK/网络依赖 |
| `p03_tests`、`p03_m1a_demo` | 自动化验证入口、可运行的业务事实演示入口 |

## 从哪里开始读

先看 [M1A 验收报告](docs/M1A_ACCEPTANCE.md)，再看 [业务不变量](docs/BUSINESS_INVARIANTS.md)、[架构对应与能力边界](docs/M1A_SCOPE.md)、[测试清单](docs/M1A_TEST_MATRIX.md)。理解和讲解可按 [源码阅读与调试](docs/LEARNING.md) 的顺序进行。

`CoreLoop::state()` 只返回 const 视图。该类的调用前提是单一调用者；它不是可被多个线程并发调用的接口。`StepResult.applied` 表示一次内存状态提交，**不表示委托发送、柜台接单或撤单成功**；必须同时检查 `error`，隔离坏证据也可能发生状态提交。

## 当前交付边界

业务内核只可在显式 fixture profile 下启动。发送维度固定为 `NotImplemented`；外部身份保持空值，不生成假的 OrderSysID。发送、EmergencyCancel、文件日志、恢复、线程运行时、本地柜台、WebBridge 的请求不会返回成功。真实 CTP 与外部对账返回 `Blocked`。

没有性能基准、盈利证明、真实行情、实盘/SimNow 联调、持久化发送保证或重启恢复保证。下一阶段必须在本次验收后另行获准。
