# 第四十三轮：完整离线预览包

日期：2026-11-07。

报告生成后将已封存 ui-source 中的静态文件与本次生成的三份数据脚本复制到运行目录 offline-preview。包内包含 index.html、runner.html、全部本地脚本/样式和数据，可以独立于 demos 工作区查看。原始 C++ JSON 和可执行文件仍保留在该运行证据目录，不复制进浏览器预览包。

bundle-manifest.json 记录运行、配置、界面清单摘要、每个包内文件的 SHA256 以及原始 JSON 引用。检查全部本地 HTML 依赖存在且留在包内，核对正常/故障、本地回放与八种报价场景的数据运行身份一致。报告和 report-sha256.json 保存包清单摘要；verify-run 在报告存在时继续核对该摘要、包文件及其静态界面与构建归档的一致性。单独包内哈希不是签名或独立可信来源。

原有三套 Node 页面交互检查增加目标目录参数，报告同时检查包内主工作台、回放和报价页面。新增四项包文件负控：修改、缺失、多出和重复记录，均仅修改内存副本。最终完整包在报告生成阶段产生，不能仅凭 C++ 构建完成就声称包已验收。

打开方式：进入最终 Release 运行的 offline-preview，打开 index.html 或 runner.html。浏览器只读离线导出状态，不需要启动 C++ 进程或预览服务器；点击来源链接可能访问外部文档。Node 检查不代表原生浏览器视觉布局验证。

Debug / Release 各 246 项 C++ 测试、27,574 次断言，0 失败、0 跳过；十二项证据负控和四项包文件负控通过。十三份诊断输出及静态界面清单逐字节一致；两份独立离线包各自通过三套页面逻辑检查。每包 16 个内容文件，另有 bundle-manifest.json；生成数据包含运行身份与生成时间，因此不要求 Debug / Release 包逐字节相同。

最终 Release 入口：[执行工作台](../evidence-local/2026-11-07-offline-bundle-release/offline-preview/index.html)、[报价轨迹](../evidence-local/2026-11-07-offline-bundle-release/offline-preview/runner.html)。当前及历史模式的完整证据验证均通过。

当前仍无真实网关、耐久交易日志或实盘发送能力。
