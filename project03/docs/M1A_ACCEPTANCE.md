# PROJECT 03 · M1A 实施验收报告

交付日期：2026-11-05。架构基线：《系统架构设计 V1.1》及用户批准的 ADR001～ADR010。授权范围：仅 M1A。交付状态：**已实施并提交验收，等待用户审查；未推进 M1B～M1F。**

## 交付结论与证据范围

本次交付是可编译、可测试的 C++20 业务内核，包含身份与业务类型、合约/交易日约束、四桶持仓、OMS 多维状态、成交证据归并、风险资源预占、资金覆盖计算，以及单线程 StepHarness。测试直接调用这些业务类和 CoreLoop，没有把架构参考模型重新打包作为实现。

最终 Debug、Release、ASan+UBSan 三种构建均通过 **85/85 个业务测试，失败 0；每种构建的聚合运行均执行 24,868 次检查**。O02 在每种构建中枚举 720 种累计/逐笔/终态事件排列。这些是正确性证据，不是 85 套互相独立的系统验收，不是性能、实盘或外部接口证据。

LSan 在当前环境无法完成 `/proc/.../task` 扫描，初始尝试中止，未获得泄漏检测结论。最终 ASan/UBSan 使用 `ASAN_OPTIONS=detect_leaks=0:halt_on_error=1`、`UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1`；不得将其表述为 LSan 已通过。

## 实际工程与执行入口

源码位于 `project03/`。核心目录是 `include/p03/`、`src/`、`tests/`、`examples/`、`docs/`、`scripts/`、`evidence/`。具体文件职责见 README 的目录表。CMake 实际构建三个目标：`p03_core`、`p03_tests`、`p03_m1a_demo`。

| 核心对象 | 实际职责 |
|---|---|
| TradingCalendar / Contract | 显式会话窗口和交易日映射、实际配置合约限定、价格步长/价格带/手数限制 |
| Order | OMS 执行、发送、撤单动作和一致性维度；Q/B/R/F 证据；数量分区和撤单历史 |
| ExecutionLedger | 经济成交键去重；正常 B 和超 Q 的整笔异常 X |
| PositionLedger | 固定启动锚点、今多/昨多/今空/昨空、逐笔记账和按桶冻结 |
| RiskEngine / FundsLedger | 最坏净仓区间、总仓、资源目标、Li/Ci 与独立预算、自成交约束 |
| CoreLoop | 唯一业务写入口，候选状态事务、不变量、未知风险停止和跨日阻断 |
| StepHarness | 逻辑时间稳定排序、保留每个输入与结果、生成可复现业务摘要 |

CoreState 不暴露可变引用；所有集成场景通过 `CoreLoop::step` 更新。单写者是当前执行器和所有权约定，不意味着 CoreLoop 能被多线程安全调用。没有新建线程、运行队列、SDK 回调、文件 Writer 或出站发送。

## 构建与测试命令

在源码工程根目录执行：

```bash
cmake -S . -B build/debug -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=ON
cmake --build build/debug --parallel 2
ctest --test-dir build/debug --output-on-failure
./build/debug/p03_tests
./build/debug/p03_m1a_demo

cmake -S . -B build/release -DCMAKE_BUILD_TYPE=Release -DBUILD_TESTING=ON
cmake --build build/release --parallel 2
ctest --test-dir build/release --output-on-failure

cmake -S . -B build/sanitizers -DCMAKE_BUILD_TYPE=Debug -DP03_SANITIZE=ON
cmake --build build/sanitizers --parallel 2
ASAN_OPTIONS=detect_leaks=0:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1 ctest --test-dir build/sanitizers --output-on-failure
```

一键收集相同类别证据：`python3 scripts/verify.py --cmake cmake --detect-leaks 0`。如使用 Ninja，可添加 `--ninja /absolute/path/ninja`。当前环境使用的完整绝对路径、退出码和编译命令分别保存在 `evidence/commands.json`、各 profile 的 `*-compile-commands.json`。源码不要求本次工作区的工具安装目录；在本机安装 CMake 和构建工具即可使用标准命令。

本次环境：Linux 6.18.57 x86_64 / glibc 2.39；GCC 13.3.0；CMake 3.31.6；Ninja 1.11.1.git.kitware.jobserver-1；lscpu 报告 AMD EPYC 9V74 虚拟 CPU、9 个在线逻辑 CPU。Debug 使用 `-g`，Release 使用 `-O3 -DNDEBUG`，Sanitizer 使用 `-g -fsanitize=address,undefined -fno-omit-frame-pointer`；共同启用 C++20、警告和 `-Werror`。没有绑核、测量负载、延迟统计或性能验收。

## 最终运行输出与失败统计

| 构建 | 聚合运行 | CTest 逐例运行 | 检查数 | 最终业务失败 |
|---|---:|---:|---:|---:|
| Debug | 85/85 | 85/85 | 24,868 | 0 |
| Release | 85/85 | 85/85 | 24,868 | 0 |
| ASan + UBSan，关闭 LSan | 85/85 | 85/85 | 24,868 | 0 |

另将源码包解压到全新临时目录，从零配置并完成 Release 构建、CTest、聚合测试和演示：85/85、失败 0。核对所有被编译的 `.cpp`、`.hpp` 与 CMakeLists.txt，均与交付源码逐字节一致；证据见 `clean-*.log`、`clean-junit.xml` 和 `clean-source-verification.json`。

三种构建的原始摘要均为：

```text
DETAIL O02 permutations=720
RESULT cases=85 failed=0 assertions=24868
100% tests passed, 0 tests failed out of 85
```

演示程序最终输出：

```text
FINAL Q=3 B=2 H=0 P=0 L=1 V=0 booked_long_today=2
```

其最后一次发送请求返回 `NotImplemented`、`applied=0`。演示从人工事实输入开始，不包含策略或模拟柜台。手工喂入真实语义的 fixture 事实不等于完成交易闭环。

测试运行器还有两个负向控制：故意失败检查返回 1；选择不存在的测试返回 2。它们均获得预期非零退出码，证明 Release 的检查没有被 NDEBUG 去掉。这些预期失败不计入业务失败。CTest 每例超时 30 秒，测试无跳过、无用空实现标绿的外部验收项。

原始证据：`debug/release/sanitizers-tests.log`、`*-ctest.log`、`*-junit.xml`、`*-build.log`、`*-demo.log`、`commands.json`、`environment.json`、`cpu.log`、`source-sha256.json`。完整 85 例清单和文件对应见 `M1A_TEST_MATRIX.md`。

## 与 V1.1 验收契约对应

以下结论限定为 **M1A 适用业务部分**。凡涉及真实外部语义、发送、文件和恢复资格，仍未验收。

| 契约 | 真实业务实现与自动化判据 | 结论 |
|---|---|---|
| A01 | 根品种/主连名、旧日、错账户、错误 tick/手数、非会话窗口拒绝；夜盘日期显式映射；非法配置不能启动 | 本地配置约束通过；动态真实合约加载未实现 |
| A04 | 数量/资金/可平量/容量/ID 耗尽及预提交失败后无半订单、半冻结、半资源；成功后一次提交 | 内存事务通过；不是持久化事务 |
| A05 | N=2、买3卖1得到 [1,5]；相反报价不能掩盖净仓限额；总仓独立检查 | 通过 |
| A06、P01 | 六类开平穿过 CoreLoop；今昨冻结竞争；成交只消费对应桶；异常恢复冻结缺口可见 | 通过；冲突态允许显式 deficit，禁止继续新单 |
| A07、O03 | Q3 / T1=1 / 撤单精确 F2 / T2=1 / 重复 T2，只释放1、记账2；资金占用不随补齐逐笔虚假下降 | 通过 |
| A08、O05～O07 | 同键同内容幂等；异内容不覆盖；超 Q 整笔进入 X，X 仍去重并计风险 | 通过 |
| O01 | 未验证、身份/日期不符、负数、超 Q 的累计证据不能提升 R，故障保守停止 | 通过 |
| O02 | 720 排列逐步检查 B、R、分区、冻结/资金和账本不变量；R>B 与 B>R 均合法 | 通过 |
| O04 | F<B、拒单后有效成交、NotSent/接单矛盾均保留事实并停止；保守资源无法满足时设置 deficit | 通过 |
| O08 | 无精确 F 时保留 H/V，不释放；查询或超时不能自动释放 | 通过 |
| O09 | 同 source/epoch 可比版本倒退、同版本异数量冲突；旧小值和不同 epoch 不伪造回调顺序 | 通过 |
| P02 | 外部/结算 fixture 观察不覆盖业务桶；真实聚合映射明确 Blocked | 分离规则通过；真实 CTP 口径未验证 |
| P03 | 旧平今风险跨日停报，原桶/epoch/offset 不变；迟到成交只应用一次；干净边界仍等待新日证据 | 阻断部分通过；换锚/新日恢复未实现 |
| P04 | 已知覆盖扣差额；部分覆盖、独立本地预算、相同扣减证据重复归属拒绝 | fixture 算术与所有权通过；外部证据取得未实现 |
| P05 | Unknown/Excluded Ci=0；错账户/epoch/币种、非法区间及未核验来源不能启用覆盖 | 通过；真实资金查询继续 Blocked |
| P06 | 订单→持仓沿用同一资源与 Ci；费用仍扣，撤单不凭本地计算增加 Aext | 保守模型通过；真实结算费用/保证金释放未实现 |

## 已发现问题及修正

开发中新增回归测试实际出现过 **2 个业务失败**：累计成交已证明部分成交后，迟到 Working 回报会回退状态；完全成交后，迟到的 Canceled/F=Q 会覆盖 Filled。两处均已修正并进入最终回归集。`development-regression.log` 保留当时 79 例、2 失败的原始运行，不拿早期失败结果冒充最终通过。

末次边界复核补上了撤单结果 Unknown 的停新单规则；后续即使动作转为 Succeeded，也不会自行恢复新单准入，既有有效成交仍继续入账。审查还补齐了撤单动作历史，防止旧动作回报覆盖最新请求；对可比累计流和动作历史设置容量；资金观察增加来源/口径/币种/区间，并以 external_deduction_id 防止一份外部扣减重复覆盖。编译阶段出现的严格告警失败已修正，最终三个 profile 均在 `-Werror` 下构建成功。LSan 环境中止独立登记，未作“修复业务代码后通过”的错误归因。

## 必要取舍与未实现边界

1. 当前一个 `p03_core` 库按文件划分类；未预建大量空服务。目录较 §15 的最终规划收敛，逻辑边界保持一致。
2. 采用候选状态复制和 noexcept swap，先保证原子更新。动态分配和 O(状态大小) 成本明确存在；没有声称零分配、无锁或任何纳秒指标。
3. 发送状态显式为 NotImplemented；Created 之后的接单/成交状态来自 fixture 输入。PendingNew、发送门闩、claim、SDK 边界和持久化资格未实现。
4. 只有固定启动种子和其后唯一成交增量；不支持新锚覆盖旧成交、快照装载、跨日恢复或自动解除停止。旧风险永远不因换日期自动消失。
5. 模型保证金对已记账开仓保守保留，后续平仓不提前返还；这会限制长时间使用预算，未来必须用可信换锚解决。SettlementObservation 不是正式结算系统。
6. 关键证据容量满会停止并保留 unknown/loss；这不是回报队列溢出后的查询覆盖恢复实现。实际 OOM、文件失败、崩溃、网络恢复和并发竞态不在本次证据内。
7. CTP SDK 精确版本、SimNow 条件和外部联调能力仍待确认。真实 CTP、LocalSim、WebBridge、真实行情/盘口、SMDP、策略、日志发送、恢复证书、事故实验室和性能测试均未实现。

ADR008 保持已批准的受限 EmergencyCancel 契约；M1A 直接请求该能力会得到 NotImplemented，不提供紧急撤单成功桩。AuditIncomplete 持久化与强制外部对账也未实现。

## 验收后的边界

请以本次实际源码、逐例日志、断言和能力状态审查 M1A。当前工作停在 M1A，不自行开展策略/LocalSim、线程运行时、日志发送或 Web。下一步只有在本次验收后，按新的明确授权进入相应阶段。
