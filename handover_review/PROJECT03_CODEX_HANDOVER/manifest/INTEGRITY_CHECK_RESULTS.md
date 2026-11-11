# 交接包完整性检查结果

检查项：18；通过：18；失败：0。必需项目原件缺失：0；保留来源原件：17。

本次执行的是资料/证据一致性自检，不编译、不运行任何C++模型/业务程序，不做外部连接，不构成M1A用户验收。

| 检查 | 结果 | 具体边界 |
|---|---|---|
| required_16_documents | PASS | 00～15全部真实存在且非空 |
| all_files_nonempty | PASS | 不以空文件冒充附件；合法文件逐个检查 |
| 17_original_bytes | PASS | 17份来源原件大小/SHA与取得时记录一致 |
| mandatory_originals_present | PASS | 必需原件缺失0；模型ZIP内含源码/输出/元数据 |
| archives_docx_crc_and_structure | PASS | 所有ZIP/DOCX CRC及DOCX正文XML有效；不是视觉等价审查 |
| adr001_to_010 | PASS | 十项背景/决定/原因/约束/状态/出处齐全，均标整理件而非历史原件 |
| full_v11_in_04 | PASS | 04包含完整原Markdown正文，不只摘要；正式原件SHA另独立核对 |
| five_p0_and_adr008_complete | PASS | 六段规范全文逐段与原件一致，含不变量、路径、反例、测试和阶段影响 |
| m1a_plan_matches_17_2 | PASS | 原17.2完整保留；候选优先审查，禁止提前后续能力 |
| model_38_names_and_hashes | PASS | 原始38名字/源码/输出哈希/5位置/720排列一致；没有重跑 |
| candidate_39_source_fingerprints | PASS | 候选39项指纹全部匹配；本轮未改业务源码/历史文档 |
| candidate_85_historical_records | PASS | 四组历史85例/JUnit/24868检查与源码一致；不代表本轮重跑或用户验收 |
| candidate_separate_evidence_identical | PASS | 独立evidence ZIP与source ZIP中的同名历史证据字节一致 |
| current_status_disambiguated | PASS | 批准时刻与当前候选分层说明；没有以旧模板抹去实现或自行验收 |
| 42_reference_entries | PASS | 原S01～S29/R01～R13全部保留；来源层级/版本/限制明确 |
| missing_materials_explicit | PASS | 必需原件与外部/历史资料缺口分开，未用空文件冒充 |
| local_markdown_links | PASS | 本地Markdown目标均存在；未把公网链接可达性冒充内容或接口实测 |
| inventory_sha256_and_coverage | PASS | {'checksummed_files': 194, 'missing': [], 'stale': [], 'self_excluded_outputs': ['manifest/FILES.json', 'manifest/FILE_MANIFEST.md', 'manifest/INTEGRITY_CHECK_RESULTS.json', 'manifest/INTEGRITY_CHECK_RESULTS.md', 'manifest/SHA256SUMS']} |

## 必须保留的状态与资料限制

- M1A candidate exists but has no formal user acceptance; approved scope remains M1A.
- Immutable historical originals contain superseded pending-review status; current overlay is explicit.
- Full native conversation export and external SDK/specification binary/connection evidence are not provided.
- LSan remains unverified; historical business tests were not rerun by this checker.

语义复核：00～15覆盖项目背景、演化、市场、完整基线、ADR/P0、实际状态、阶段、测试、四事故、七页Web、性能可靠性、带教、未决和工作规则。此为人工索引审读+上述结构/字节检查，不宣称形式化证明LLM理解全部历史。
DOCX按ZIP/正文XML及原件指纹验证；本轮没有修改DOCX，也没有逐页视觉排版或跨格式逐句等价验收。
对旧原件里的待审/未实现文字，采用后续用户批准和当前证据的时间层次覆盖，原件字节不改。
缺失资料详情见MISSING_MATERIALS.md。用户要求的项目报告/模型原件齐全；未提供的外部与原生对话资料不补造。
