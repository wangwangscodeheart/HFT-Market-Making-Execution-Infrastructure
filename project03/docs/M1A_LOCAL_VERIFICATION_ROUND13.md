# 第十三小轮：M1A 本地证据门禁与源码归档

日期：2026-11-06。结论：当前枚举的 M1A 本地回归门禁通过，可按用户最新授权推进限定范围的 M1B 本地闭环工程；这不是外部接口、Linux、并发或生产就绪结论。

## 实际结果

两次全新目录构建：MSVC C++20 Debug、Release，各 140/140、25645 次断言、0 失败、0 跳过。源码 TEST_CASE 声明、二进制 --list 注册表、聚合 PASS/断言统计与 CTest JUnit 四方逐项相同。

具名业务证据覆盖 A01、A04～A08、O01～O09、P01～P06 的本地适用部分；类型/边界补充回归另列，不冒充 J/K/Q 的实际 Writer、门闩或恢复运行时测试。完整清单见 M1A_CURRENT_TEST_MATRIX.md，逐例有源码位置和实际断言数。

两类演示共 14 步快照与各自文本 Demo 摘要相同，Debug/Release JSON 字节相同；工作台交互逻辑检查通过。没有新增原生浏览器截图验证。

## 修复的证据链缺口

原构建脚本允许复用目录，预览脚本未强制关联构建时源码与产物；因此旧成功日志有可能被当成修改后源码的当前结果。

现在每次构建使用全新运行目录，已有目录直接拒绝。开始时记录源码指纹并逐文件归档，结束后重算指纹；构建期间改变源码则整轮记录为 failed。成功完成配置、编译、CTest、聚合测试、失败退出码负控和两类演示后，才发布 completed 状态。

完成清单包含工具链记录、JUnit、测试列表、日志、快照、三个可执行文件、CMakeCache 等 19 个产物哈希。原始日志和产物保留；下游生成报告/预览之前，必须同时核验完成状态、源清单哈希、归档内容、当前源码及产物内容。

这是完整性和一致性检查，不是抵御任意恶意篡改的数字签名。归档的是本地代码输入，仍需匹配工具链/SDK 才能重建；不承诺跨时间二进制逐字节一致。

## 负控

- 8 个内存负控：失败状态、非零退出、源码变动、产物变动、缺产物、重复记录、缺源码和新增未构建源码，全部被拒绝，不修改历史文件。
- 实际复用已完成目录：返回非零，原 run-status 哈希未变。
- 实际指定不存在的编译器：配置失败，整轮状态为 failed；证据校验再次拒绝该轮。
- 测试运行器真实断言失败返回 1，不存在用例返回 2，纳入构建完成门禁。

## 证据位置

- evidence-local/2026-11-06-m1a-sealed-debug/
- evidence-local/2026-11-06-m1a-sealed-release/
- 预期失败对照：evidence-local/2026-11-06-m1a-build-failure-control/

每个成功目录含 source/ 源码归档、build-source-manifest.json、run-status.json、m1a-evidence-report.json、m1a-test-matrix.md 和原始日志。

## 新复现方法

必须使用新运行名，避免覆盖历史证据。工作区根目录的 PowerShell：

```powershell
$verificationRun = 'm1a-release-' + [DateTime]::UtcNow.ToString('yyyyMMdd-HHmmss')
powershell.exe -NoProfile -ExecutionPolicy Bypass -File .\project03\scripts\build-msvc.ps1 -RunName $verificationRun -Configuration Release
if ($LASTEXITCODE -ne 0) { throw 'Build failed; do not prepare preview' }
node .\project03\scripts\report-m1a.cjs $verificationRun
```

不指定 RunName 时构建器自动生成 UTC 时间与随机后缀的独立目录。报告自动执行证据负控、生成预览数据和交互逻辑检查。报告本身也不覆盖同一运行已有报告。

历史 v2 封存记录可用 `node scripts/verify-run.cjs RUN_NAME --historical` 核验归档与产物；该模式明确不比较当前工作区，不能用它为修改后源码出当前报告。更早的非封存历史日志仍保留，但不能绕过新证据门禁重新发布为当前结果。

## 已知限制与后续

本机仅找到可用 MSVC；VS LLVM 目录只有格式化/静态分析工具，没有 clang 编译器，WSL 没有已安装发行版，也未发现 Docker/GCC。当前源码未在 Linux、GCC/Clang、Sanitizer 或真实 SDK 上复验；既往候选的 Linux 日志不替代本轮验证。

接下来推进 M1B 的确定性本地对端与独立查询，继续采用有来源的正常订单流程和明确标注的派生输入。M1C/D 的真实并发、gate、文件耐久性及恢复仍各有独立工程检查；不因为本地门禁通过而启用真实交易。
