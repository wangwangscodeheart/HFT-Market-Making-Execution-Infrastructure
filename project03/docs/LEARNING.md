# 读懂、调试和讲解 M1A

先看“事实如何改变状态”，再看容器和 C++ 写法。当前不需要先学复杂做市定价，也没有真实交易接入。

## 一条可独立讲清的路径

打开 `examples/m1a_demo.cpp`：手工给参考价，提交买开 3 手；T1 成交 1；请求撤单；精确终态证明最终成交 2；T2 迟到 1；再重复 T2。最终 **B=2、H=P=V=0、L=1、今多=2**。最后请求发送，明确得到 NotImplemented。

这里“成交悬置”的意思是：柜台语义的证据说已经成交了，但详细成交记录还没拿齐。不能当作仍可用资金，也不能凭空造逐笔。`H` 占着风险，等唯一逐笔到来才转进 `B` 和四桶。

## 源码阅读顺序

| 顺序 | 文件与关键入口 | 应能解释的问题 |
|---|---|---|
| 1 | `types.hpp`、`reference_data.cpp` | 为什么订单 ID 和撤单动作 ID 不用一个 uint64_t？为什么夜盘日期不能直接当交易日？ |
| 2 | `oms.cpp` 的 `parts/observe_cumulative/observe_terminal/book_trade` | 为什么订单显示终态仍可能占用风险？为什么 R 可以大于 B，也允许 B 大于 R？ |
| 3 | `ledger.cpp` 的 `classify/insert` | 为什么 queryId 和推送通道不进成交经济键？同键异内容如何处理？ |
| 4 | `positions.cpp` 与 `core.cpp::recompute` | 买开、卖平今、卖平昨分别动哪个桶？何时释放冻结？ |
| 5 | `risk.cpp` 与 `funds.cpp` | 买卖报价为什么不能先对冲？Li/Ci 如何避免资金重复扣减？ |
| 6 | `core.cpp::step/apply/invariant` | 候选状态为何能保证失败无半份冻结？为什么这是正确性方案而非性能结论？ |
| 7 | `step_harness.cpp` 与对应测试 | 相同逻辑时间怎样排序？为什么可复现状态不等于可复现物理延迟？ |

## 三个调试入口

先执行 README 的 Debug 构建，然后：

```bash
gdb --args ./build/debug/p03_tests --case O03_A07_cancel_exact_and_late_trade_release_only_one
```

在 GDB 中：

```text
break p03::Order::observe_terminal
break p03::Order::book_trade
run
print booked_
print lower_bound_
print intent_.quantity
next
continue
```

观察终态到达和 T2 到达前后的字段；不需要背字段数值，能由 Q/B/R/F 推导出分区即可。`p03::CoreLoop::recompute` 可观察四桶冻结和资金资源怎样跟随同一订单变化。

## 可以用于面试的准确介绍

“第三个项目目前完成 M1A。我用 C++20 做了单写者内存交易业务内核：强身份、显式合约和交易日约束、期货今昨多空四桶、OMS 多维状态、成交去重与风险预占。新单在候选状态里同时建立订单和资源，失败整体不提交。累计成交证据先形成悬置，唯一逐笔到来后才记仓。撤单终态和迟到成交冲突时保留事实、停止新单并恢复保守占用。现在有真实业务类的自动化测试和确定性输入脚手架；CTP、本地柜台、发送日志、并发运行时和 Web 都还没实现，所以我不会宣称已经完成低延迟交易平台。”

常见追问包括：copy/swap 有何成本、终态为何还可更新、为什么 TradeID 的真实范围要再核验、未知状态为什么不能重发、为什么本次不能报 P99.9。答案都应回到具体类和测试。后续再逐阶段补齐运行时与真实接口，不把尚未实现的规划当作源码能力。
