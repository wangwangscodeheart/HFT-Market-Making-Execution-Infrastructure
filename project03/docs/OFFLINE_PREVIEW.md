# 第四小轮：C++ 离线快照驱动工作台

本轮不实现 WebBridge。数据路径是：共享场景输入 → C++ StepHarness/CoreLoop → snapshot.json → 校验与嵌入 → 浏览器只读展示。

`examples/demo_scenario.hpp` 是文本 Demo 和快照导出的唯一场景定义。`export_snapshot.cpp` 在每个事件之后读取真实业务状态，包括订单证据分解、持仓、冻结、资金需求、去重结果、不变量和状态摘要。没有解析或重算核心业务规则。

运行命令（工作区根目录）：

```powershell
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\project03\scripts\build-msvc.ps1 -RunName 2026-11-05-offline-export
node .\project03\scripts\prepare-preview.cjs 2026-11-05-offline-export
Start-Process .\demos\round02\index.html
```

实际结果：MSVC Debug 构建成功；85/85 测试，24,868 断言；八步 JSON 与文本 Demo 的状态摘要逐步完全一致。重复 t2 返回 Duplicate，持仓及资金需求不变；发送请求返回 NotImplemented；每步不变量满足。

生成文件 `demos/round02/core-snapshot.js` 包含运行目录、UTC 生成时间、原 JSON 与可执行文件 SHA256，以及源文件指纹。缺少快照时页面报错，不回退到手写订单数据。浏览器展示历史快照，不向核心提交命令，也没有持续运行的核心进程。K 线仍是独立构造的说明样本。

新原始证据：`evidence-local/2026-11-05-offline-export/`，没有覆盖上一轮日志。前端仅做脚本语法及数据一致性检查，未完成自动化浏览器排版验证。

## 业务解释

委托请求不是成交。买开 3 手时，需要为全部可能成交的数量保留资源；成交 t1 后，部分资源从订单风险转到持仓风险，不会凭空消失。

撤单终态报告累计成交 2 手，但逐笔只有 1 手时，不能释放差额；先将其标为暂记，等待迟到成交补齐。重复成交必须幂等，即看到第二次也不能再次增加持仓。

本样例初始资金需求为 306，终态后为 204；这是配置的每手保证金测试量 100 与费用测试量 2 的计算结果，不是真实人民币保证金或 CTP 试算。

目前无需额外金融资料。接入真实合约前需要运行日有效合约目录、交易日历、合约参数；接入 CTP 前需要官方 SDK 精确版本、接口说明和目标环境能力证据，届时逐项列出，不从教学样本推断柜台行为。
