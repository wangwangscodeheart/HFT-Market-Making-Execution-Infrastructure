# PROJECT 03 · Codex 接管入口

本包将已批准的中国市场交易系统设计、原件、历史证据和当前实际状态一起迁移；它不是新的架构方案，也不授予后续阶段权限。

## 先读这五句话

1. 项目是 **HFT Market Making & Execution Infrastructure**：中国真实市场优先、C++ 系统工程为主线，用简化双边限价报价产生完整交易链路负载；不以收益或金融网页为目标。
2. 市场与接口已批准：**上期所商品期货 / RB，实际合约动态确认；普通账户语义；CTP C++ TraderAPI + MdAPI；本地确定性模拟优先，具备条件后 SimNow；SMDP 2.0 文档 V1.22 为独立本地协议实验。**
3. 正式设计基线是 [完整 V1.1](04_ARCHITECTURE_BASELINE.md) 与 ADR001～ADR010。V1.0 是历史稿；原 V1.1 文首“待审”是提交时状态，已被后续用户正式批准覆盖，不得改写原件伪装原始版本。
4. **当前状态：M0 完成，V1.1 架构正式批准，允许启动 M1A。** 这是批准/授权边界，不代表 M1A 已正式验收，也不允许进入 M1B～M1F。
5. **当前实际进度补充：已有 M1A 候选实现和原始测试记录，尚无用户正式验收通过。** “M1A 尚待实际工程实施和验收”适用于批准启动时刻；若用它描述当前代码存在性会与证据冲突。本包保留候选实现，不抹去工作，不把候选标为完成验收。详见 [状态表](07_IMPLEMENTATION_STATUS.md)。

## 目前允许与禁止

本轮只整理资料、保存原件、静态核对和交接包自检；不修改 M1A 业务代码。VS Code Codex 接管时先阅读、校验、审查及复现候选，再按用户给定的 M1A 任务处理；不重新搭一套相互冲突的工程。

禁止跳阶段、改 ADR、自动连接 CTP/SimNow、写 Web/真实线程/持久化发送、以参考模型代替业务实现验收、虚报连接/性能/盈利。受限 EmergencyCancel 只有已批准的 ADR008 契约，当前候选没有该发送实现。

## 最短阅读路径

| 顺序 | 文件 | 目的 |
|---|---|---|
| 1 | [01 背景](01_PROJECT_CONTEXT.md)、[02 演化](02_PROJECT_HISTORY.md) | 知道为什么做、哪些历史方案已被替代 |
| 2 | [03 市场接口](03_MARKET_AND_PROTOCOL.md) | 区分交易所协议、柜台 API、内部接口及待确认能力 |
| 3 | [04 完整架构](04_ARCHITECTURE_BASELINE.md) | 必须阅读全文，不只看摘要 |
| 4 | [05 ADR](05_ADR_REGISTER.md)、[06 P0](06_P0_CONTRACTS.md) | 锁定决策与五项安全边界 |
| 5 | [07 状态](07_IMPLEMENTATION_STATUS.md)、[08 M1A](08_M1A_EXECUTION_PLAN.md)、[09 验收](09_TEST_AND_ACCEPTANCE.md) | 区分批准、候选实现、历史测试与正式验收 |
| 6 | [14 风险](14_OPEN_QUESTIONS_AND_RISKS.md)、[15 工作规则](15_CODEX_WORKING_RULES.md) | 防止能力被猜成已支持 |
| 按主题 | 10～13 | 事故、Web、性能可靠性和教学要求，均属于长期范围 |

## 第一次操作

在本交接目录执行 `python3 tools/verify_handover.py`，只校验文档与证据，不运行交易程序。检查 [完整性结果](manifest/INTEGRITY_CHECK_RESULTS.md) 和 [缺失资料](manifest/MISSING_MATERIALS.md)。需要复现 M1A 时，按第 09 文件在独立 build 目录构建，原始 ZIP 和历史日志保持只读，新输出另存，不覆盖历史证据。

可将 [CODEX_FIRST_MESSAGE.md](CODEX_FIRST_MESSAGE.md) 作为 VS Code Codex 第一条任务。默认先输出接管核对结果、待审项与复现结果，不自行宣告 M1A 通过或进入 M1B。
