# HFT Market Making & Execution Infrastructure

PROJECT03：C++20 买方交易执行基础设施。本仓库保存当前源码、测试、原始历史资料与封存证据。M1A 未获正式用户验收，M1C 尚未完成；不代表生产就绪或已接入 CTP。

完整背景、正式 V1.1、ADR、实现状态和恢复步骤见 [总报告](PROJECT03_COMPLETE_ARCHIVE_AND_RESUME_REPORT.md)。本仓库是封存后的新 Git 起点，不是此前开发的历史提交记录。

用户提供的[长期发展路线与未来接管展望报告（V1.0-DRAFT）](PROJECT03_LONG_TERM_ROADMAP_AND_HANDOVER_OUTLOOK.md)按附件原文保存，作为规划参考，不变更架构或授权新开发。草案提及的三份旧封存报告已整合进上述总报告，原件可从封存 ZIP 取得；架构位于 `project03/docs/architecture/V1.1-approved.md`，ADR 位于 `handover_review/PROJECT03_CODEX_HANDOVER/adr/`。

## 完整封存 ZIP

原始 ZIP 超过 GitHub 个人账户 LFS 单文件上限，按原始字节拆为 Git LFS 分片，原 ZIP 未重新压缩或修改。克隆后恢复：

```powershell
git lfs pull
powershell -NoProfile -ExecutionPolicy Bypass -File archive-parts/restore.ps1
```

分片顺序和 SHA-256 见 [manifest.json](archive-parts/manifest.json)。恢复脚本拒绝覆盖已有 ZIP，并核验分片与最终 ZIP。原 ZIP 的 SHA-256 为 `6faed74c9f68ec8869dbb7c8c04ecef8d468d82c40118d604c8b5f55152f3e4a`。

历次 `project03/evidence-local` 原始日志、来源归档和结果一并保存。构建缓存及可重建二进制已清理；`preserved-build-evidence` 保留从缓存中提取的原始日志和配置。恢复开发需新运行名重新构建，不能用缺少产物的旧目录宣称核验通过。
