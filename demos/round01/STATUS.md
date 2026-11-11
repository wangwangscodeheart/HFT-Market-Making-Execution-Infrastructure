# 第一轮结果

## 本地界面与说明 Demo

`index.html` 提供三种风格切换和八步业务说明；全部是教学 fixture。
`node check.cjs` 检查步骤导航、数量总和、主题切换及重置，不验证浏览器排版或 C++ 业务。

## 真实 C++ Demo 复现

已尝试使用本机 VS2022 的 MSVC，以 C++20、/EHsc、/utf-8、/W4 编译原始候选的全部 src/*.cpp 与 examples/m1a_demo.cpp。

结果：编译失败，原始 `src/core.cpp:114` 的
`static_assert(std::is_nothrow_swappable_v<CoreState>)` 不满足。

完整输出保存在 `build/build.log`，编译参数保存在 `build/compile.rsp`。
未生成可运行 Demo，未运行候选业务测试。此次结果不能解释为候选在其原 Linux 工具链上失败，也不能将历史测试记录算作本机验证通过。

不删除静态断言、不降低异常安全要求、不修改原始交接包源码。
下一小轮先定位 CoreState 成员在 MSVC 下的 noexcept swap / move 特性，再做最小可审查修正并复现。

## 视觉验证限制

本地 HTTP 返回 200，JS 语法与说明导航检查通过。内置浏览器访问宿主不可用，尚未完成实际浏览器截图和排版检查。

## 本轮边界

只新增 demos/round01。未改 ADR、候选业务源码或历史日志。
多合约、多品种目前仅在界面说明中标明未来归属维度，未实现扩展业务。
