# 独立参考模型附件

originals/保留两个哈希相同的原ZIP，expanded/按原ZIP展开4个真实文件。
源码：expanded/PROJECT03_V1.1_contracts/contract_model_tests.cpp。
model_results.txt与execution_metadata.json是历史实际运行记录，不是本轮生成的新结果。
38具名检查、5个Kill位置、720排列只验证有限模型；不接SDK，不运行真实mutex/线程/fsync/队列。
详见../09_TEST_AND_ACCEPTANCE.md。接管若重跑，输出另存，不覆盖原件；失败不能运行旧二进制。
