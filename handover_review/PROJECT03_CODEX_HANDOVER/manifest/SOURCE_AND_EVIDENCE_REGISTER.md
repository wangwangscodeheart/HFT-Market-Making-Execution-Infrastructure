# 来源与证据登记

本表区分官方技术事实、项目设计、模拟假设与实际运行。原M0的S01～S29、V1.1的R01～R13全部保留，不把二手资料写作官方规范。来源“已核验”需看具体层次：报告当时核验、本轮链接/要点抽查、当前环境实测是不同事情。

## 重要事实/结论

| 编号 | 事实/结论 | 来源文件/章节/页码 | 来源类别/是否官方 | 核验状态 | 设计决策？ | 模拟假设？ | 仍待确认 |
|---|---|---|---|---|---|---|---|
| F01 | 中国真实市场/真实规范优先，C++系统工程而非策略研究 | history/USER_DECISION_RECORD.md C01；01 | 用户要求 | 已确认 | 是 | 否 | 否 |
| F02 | SHFE/RB普通账户限价报价，CTP+本地→SimNow，SMDP独立 | history/USER_DECISION_RECORD.md C02；M0 §1/§5 | 用户正式决策；官方依据另登记 | 已批准 | 是 | 否 | 外部能力待核验 |
| F03 | CTP是柜台客户API，不是所有交易所统一官方协议 | M0 §5.1/S06/S19；V1.1 §2.1/§4 | 厂商/经纪机构一手资料 | 原报告核验；本轮保留 | 接口路线是 | 否 | 当前SDK组合待确认 |
| F04 | RB单位/价格步长等固定条款与运行日临时参数分开 | M0 §4.1/S01～S05/S18；V1.1 R01/R02 | 交易所官方 | 按原报告所列版本；非全部现行性重验 | 建模是 | 否 | 运行日目录/参数仍待确认 |
| F05 | 普通A股回转与部分ETF区别，不能所有ETF一概T+0 | M0 §3.2/S10 | 上交所规则 | 原报告所列2026版；本轮未新法律判断 | 市场选择依据 | 否 | 具体证券/账户须再核验 |
| F06 | 普通账户无专业做市商资格/特殊待遇默认授权 | M0 §4.2/S15；用户C02 | 交易所办法+用户业务边界 | 原报告版本保留；不授予资格 | 是 | 否 | 任何实际权限须确认 |
| F07 | CTP当前SDK精确版本、ABI、模式未冻结 | M0 §5.2；V1.1 §18.1；14 U01/U02 | 项目核验状态 | 缺可校验当前包 | 是 | 否 | 是，不阻塞本地M1A |
| F08 | SimNow只是后续许可范围功能联调，未连接 | M0 §5.4/S09/S26；用户C02 | 交易所介绍+项目状态 | 定位有来源；当前连接未测 | 是 | 否 | 是 |
| F09 | CTP有限档位/快照不默认MBO、逐笔带、序号和固定频率 | M0 §5.3/S19；V1.1 §10.1 | 历史厂商指南+保守能力边界 | 当前环境能力未测 | 是 | 否 | 是 |
| F10 | SMDP 2.0接口文档V1.22、独立本地规范实验 | M0 §5.1/S04；V1.1 §10.2/R05 | 交易所官方发布目录 | 本轮目录可达，原PDF抓取超时 | 是 | 未来夹具是构造输入 | PDF/字段与夹具覆盖仍待确认 |
| F11 | 文档V1.22与报文版本字段不同；MBP切片非MBO | M0 §5.1/§5.3；V1.1 §10.2 | 按已归档报告的规范研究记录 | 本轮未重新逐字段审查 | 是 | 否 | 实际编码前复核规范 |
| F12 | 单写者、统一端口、有限模块/线程与独立WebBridge | V1.1 §1～§5/§14；ADR001/002/003/009 | 项目设计 | 已批准，不是交易所强制架构 | 是 | 否 | 真实线程/桥接尚未实现 |
| F13 | J前缀/Writer故障/旧意图禁重发 | V1.1 §12.1～12.4；ADR006；06 | 项目安全契约+write/fsync手册 | 已批准；真实Writer未实现 | 是 | 模型有限验证 | M1D实现验证 |
| F14 | claim与TryCommitCall、SDK入口不同，Kill三状态 | V1.1 §5.3/5.4；ADR007；06 | 项目安全契约+C++同步参考 | 已批准；模型5位置 | 是 | 模型调度是构造 | 真实并发M1C |
| F15 | 可信R/F、H/P/L/V、B与X、查询推送去重 | V1.1 §7.3～7.5；ADR005 | 项目业务安全契约 | 已批准；模型和候选分别有记录 | 是 | fixture来源明确 | CTP证据等级未核验 |
| F16 | 业务四桶不等于外部/结算持仓，旧日未知不搬桶放行 | V1.1 §9.1～9.6；P01～P03 | 项目业务契约+历史厂商语义 | 已批准；候选只阻断/固定anchor | 是 | fixture观察是 | 真实映射/换锚待确认 |
| F17 | Li/Ci覆盖唯一归属，Unknown=0，外部与本地预算分别检查 | V1.1 §9.5；P04～P06；候选funds.* | 项目保守资金模型 | 设计批准；候选算术记录存在 | 是 | 金额/保证金/费用均fixture | 真实柜台口径未核验 |
| F18 | 丢回报不能靠队列空/登录复位，需同代覆盖证书先持久化 | V1.1 §12.6；Q01～Q09 | 项目恢复安全契约 | 已批准；模型有记录；真实恢复未实现 | 是 | LocalSim快照能力将另定义 | CTP覆盖Unknown |
| F19 | ADR008只可靠已有订单减险，AuditIncomplete永久历史 | 用户C03/C04；V1.1 §8.4、Q08 | 用户正式批准的项目契约 | 已批准；发送未实现 | 是 | 模型检查不等于发送 | 真实适配/审计恢复待实现 |
| F20 | 38模型检查/5位置/720排列，非真实系统验收 | reference_model/expanded/PROJECT03_V1.1_contracts/{CPP,结果,元数据}；用户C04 | 项目原始代码/运行记录 | 源码与输出SHA核对；本轮未编译 | 否，测试事实 | 有限模型 | 真实实现仍需各阶段验收 |
| F21 | M1A候选已有85具名测试，三配置及干净构建85/85 | implementation/originals；候选tests/与evidence/* | 项目源码与历史运行日志 | 本轮核对源码声明/JUnit/输出/哈希；未重跑 | 否，实施事实 | 全部为fixture业务材料 | 用户正式验收仍待 |
| F22 | LSan未通过，ASan/UBSan最终detect_leaks=0 | 候选evidence/sanitizers-lsan-blocked.log、environment.json | 实际历史错误/配置 | 日志可核对；不是业务错误根因证明 | 否 | 否 | 适用环境补测 |
| F23 | Knight历史部署/异常子订单，非本项目结果 | M0 §11.1/S20；V1.1 R06；10 | SEC行政命令 | 本轮抽查PDF第6页15～17段及控制段 | 实验设计是 | 未来输入是构造 | 事故实验未运行 |
| F24 | Nasdaq官方初步更新说明连接/流量/冗余问题，未归因HFT | M0 §11.2/S21；V1.1 R07；10 | Nasdaq官方初步更新 | 本轮核对Summary of Preliminary Findings | 实验设计是 | 本地缺口是独立构造 | 实验未运行 |
| F25 | 光大重下错误倍增及回报后额度控制 | M0 §11.3/S22；V1.1 R08；10 | 证监会2013-08-30原文 | 本轮核对技术设计缺陷段 | 实验设计是 | 期货倍增是构造 | 实验未运行 |
| F26 | 上交所2024确认缓慢、11:13起逐步恢复；两公告不证明内部根因 | M0 §11.4/S23；V1.1 R09；10 | 上交所31/32号公告 | 本轮抽查正文；未穷尽后续公告 | 实验设计是 | 延迟回报是构造 | 内部原因/实验结果不补造 |
| F27 | 七页工作台、状态投影、命令幂等与浏览器隔离 | V1.1 §14；M0 §10；ADR009；11 | 项目设计 | 已批准，未实现 | 是 | 不能随机数据冒充 | M1E/M5实施 |
| F28 | 性能必须实测，当前无P99.9等数字 | M0 §12；V1.1 §16；12 | 用户要求+测量方法一手参考 | 方法保留；当前无基准结果 | 是 | 不预填 | M3/长稳后续 |
| F29 | 教学从业务到机制源码，用户需独立调试解释 | 用户C01；M0 §15；13 | 用户要求 | 已确认，不从旧年级推断现状 | 工作约束 | 否 | 按理解持续调整 |
| F30 | 当前只批准到M1A，候选未有正式验收，交接本轮不编码 | 用户C04/C06；00/07/15 | 用户授权+当前会话 | 已确认；不能旧模板抹掉源码 | 是 | 否 | M1A审查/后续授权待用户 |

## 原始来源逐项登记（版本和限制按原件保留）

此处完整保留来源条目的原报告说明，而不是声称本轮重验了全部附件。原件引用中的私人Library链接另有本地映射，不是唯一阅读入口。

| 编号 | 名称 | 来源文件/行 | 一手/官方类别 | 本轮核验 | 原报告版本、范围、限制和入口 |
|---|---|---|---|---|---|
| S01 | 上期所螺纹钢期货业务细则。 | market_and_protocol/PROJECT03_M0_V1.0.md:510 | 交易所官方发布/目录（目录不等于完整附件审查） | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 2026-01-01 发布并施行；已读正文。适用 RB 期货，支持单位、价格步长、月份和到期等业务结论。[官方正文](https://www.shfe.com.cn/regulation/exchangerules/productrules/202512/t20251231_829962.html) |
| S02 | 上期所交易时间。 | market_and_protocol/PROJECT03_M0_V1.0.md:512 | 交易所官方发布/目录（目录不等于完整附件审查） | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 动态页面；已读 RB 所在时段及交易日说明。临时节假日公告优先，页面未给独立版本号。[官方页面](https://www.shfe.com.cn/services/calenderandholidays/tradinghours/) |
| S03 | 上期所交易规则。 | market_and_protocol/PROJECT03_M0_V1.0.md:514 | 交易所官方发布/目录（目录不等于完整附件审查） | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 官网发布日期 2026-06-12；已读相关正文。适用上期所，支持交易日、当日有效、撮合优先原则及持仓限额的背景。[官方正文](https://www.shfe.cn/regulation/exchangerules/rules/202606/t20260603_831934.html) |
| S04 | 上期所第二代行情发布平台接口规范 V1.22。 | market_and_protocol/PROJECT03_M0_V1.0.md:516 | 交易所官方发布/目录（目录不等于完整附件审查） | 本轮核对V1.22官方发布目录；PDF抓取超时，未重新逐字段审查/保存原PDF | 正文封面 2026-07-21，发布页 2026-07-28；已读协议正文相关章节。适用 SMDP 2.0，重点章节 2、4、5、6、7；不等于 CTP MdAPI 规范，不证明本项目有行情许可。[官方 PDF](https://www.shfe.com.cn/services/technology/technical_download/202607/P020260728318635042109.pdf) |
| S05 | 上期所交易管理办法。 | market_and_protocol/PROJECT03_M0_V1.0.md:518 | 交易所官方发布/目录（目录不等于完整附件审查） | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 2026-09-14 发布并施行；已核对指令、集合竞价及附则。具体下单量可另行公告调整。[官方正文](https://www.shfe.com.cn/regulation/exchangerules/otherrules/202609/t20260914_833388.html) |
| S06 | 上期技术 CTP 软件主用业务介绍。 | market_and_protocol/PROJECT03_M0_V1.0.md:520 | 上期技术产品介绍（官方域托管）；供应商产品资料而非交易所协议 | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 官方产品动态页，无独立版本号；已读。支持供应商与柜台角色区分。页面上厂商宣称的性能不作为本项目测量结果。[官方页面](https://www.shfe.com.cn/sfit/memberbusiness/ctpsoftwareproduct/ctpsoftwaremain/) |
| S07 | 中信期货信息系统外部接入。 | market_and_protocol/PROJECT03_M0_V1.0.md:522 | 期货公司官方说明；不是交易所统一协议 | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 期货公司官方动态页；已读环境、API 下载及生产接入审核说明。地址与参数未实际连接，不能据此保证可用。[官方页面](https://www.citicf.com/e-futures/csc/app/external_access) |
| S08 | 中信期货开发专栏。 | market_and_protocol/PROJECT03_M0_V1.0.md:524 | 期货公司官方说明；不是交易所统一协议 | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 官方动态页展示 CTP V6.6.8_20220712 与易盛 V10 1.0.1.24-20250721 等资料入口；已读页面，未成功校验下载包。它们是该页明示版本，不是本项目已冻结版本。[官方页面](https://www.citicf.com/e-futures/csc/app?type=7) |
| S09 | 上期所关于 SimNow 仿真交易环境的介绍。 | market_and_protocol/PROJECT03_M0_V1.0.md:526 | 交易所官方发布/目录（目录不等于完整附件审查） | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 2021-08-04；已读。证明平台身份与仿真定位，当前服务窗口及准入条件待确认。[官方介绍](https://www.shfe.com.cn/services/indexopt/promotion/202108/t20210804_797695.html)；[平台入口](https://www.simnow.com.cn/)；[API 下载入口](https://www.simnow.com.cn/static/apiDownload.action)。本次后两个入口存在访问超时或拒绝，未完成注册或下载验证。 |
| S10 | 上交所交易规则 2026 年修订。 | market_and_protocol/PROJECT03_M0_V1.0.md:528 | 交易所官方发布/目录（目录不等于完整附件审查） | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 上证发〔2026〕41 号，2026-04-24 发布，2026-07-06 施行；已核对官方发布内容及第 3.1.4、3.1.5 条。继续暂缓实施条文须同时注意，不能将全部新条文视为无条件生效。[官方正文与附件入口](https://www.sse.com.cn/lawandrules/sselawsrules2025/trade/universal/c/c_20260424_10816492.shtml) |
| S11 | 中泰证券 XTP 与 XTP-Pro 文档。 | market_and_protocol/PROJECT03_M0_V1.0.md:530 | 券商/接口提供方官方资料 | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 官网动态资料；主页展示传统 XTP SDK 2.2.39.3，XTP-Pro 文档另有迁移与接口说明，未冻结 Pro 版本。已核验产品与文档入口；测试账号、具体功能权限和可达性未测试。[XTP 官网](https://xtp.zts.com.cn/)；[XTP-Pro 官方文档](https://xtp.zts.com.cn/xtp-pro/) |
| S12 | 上交所技术接口专区。 | market_and_protocol/PROJECT03_M0_V1.0.md:532 | 交易所官方发布/目录（目录不等于完整附件审查） | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 目录列 IS122 竞价 STEP/Binary 0.58 版、IS120 行情 STEP/Binary 0.63 版，日期 2026-09-18。已核验发布目录，未逐字段审查这些候选接口附件。[官方目录](https://www.sse.com.cn/services/tradingtech/data/) |
| S13 | 证监会期货市场程序化交易管理规定试行发布说明。 | market_and_protocol/PROJECT03_M0_V1.0.md:534 | 监管机关官方原文 | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 2025-06-13 发布，自 2025-10-09 实施；已读发布说明。适用于期货市场，不能将证券市场高频阈值直接搬来使用。[官方说明](https://www.csrc.gov.cn/csrc/c100028/c7564353/content.shtml) |
| S14 | 上期所关于程序化交易有关事项的通知。 | market_and_protocol/PROJECT03_M0_V1.0.md:536 | 交易所官方发布/目录（目录不等于完整附件审查） | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 上期发〔2025〕232 号，2025-08-08；已核验报告范围与过渡安排。报告时点在 2026 年，不能继续使用过期过渡期假设。[官方通知](https://www.shfe.cn/publicnotice/notice/202508/t20250808_828608.html) |
| S15 | 上期所做市商管理办法。 | market_and_protocol/PROJECT03_M0_V1.0.md:538 | 交易所官方发布/目录（目录不等于完整附件审查） | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 2026-09-11 发布并施行；已读资格、协议及专用编码相关条款。适用经交易所认可的做市商，不授予本项目或普通账户资格。[官方正文](https://www.shfe.com.cn/regulation/exchangerules/otherrules/202609/t20260907_833239.html) |
| S16 | 郑商所白砂糖期货业务细则。 | market_and_protocol/PROJECT03_M0_V1.0.md:540 | 交易所官方发布/目录（目录不等于完整附件审查） | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 2024-06-26 公告修订版，部分条款 2024-12-01 实施；已读正文。用于候选研究，未确认其后有无整体修订，不作为当前实接参数冻结依据。[官方 PDF](https://www.czce.com.cn/cn/uploadfile/2024/06/26/20240626165544343.pdf) |
| S17 | 易盛 API 接入。 | market_and_protocol/PROJECT03_M0_V1.0.md:542 | 接口供应商官方资料 | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 官方页面列 V10 交易 1.0.1.19、行情 1.0.0.7，更新 2024-07-18；已核验页面中的测试和授权说明。当前最新版及可登录性待确认。[官方说明](https://www.esunny.com.cn/market/info/155) |
| S18 | 上期所交易投教说明。 | market_and_protocol/PROJECT03_M0_V1.0.md:544 | 交易所官方发布/目录（目录不等于完整附件审查） | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 动态页；已核验交易指令与数量说明。实施时仍核对具体合约元数据和最新公告。[官方页面](https://www.shfe.cn/specialtopic/investor/trade/) |
| S19 | 上期技术 CTP 客户端开发指南。 | market_and_protocol/PROJECT03_M0_V1.0.md:546 | 厂商原著、期货公司官网托管；历史指南 | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 作者为上海期货信息技术有限公司，东方财富期货官方网站托管；正文修订表 V3.1、2015-10-16，文件名含 2016-10-27。已读相关章节，仅作历史基本语义依据，不是当前 SDK 全量规范。[官方期货公司托管 PDF](https://www.eastmoneyfutures.com/software/9809db08-c8cb-4cc4-b631-8a16f0c2dfb9/0f757a06-10a5-48c2-817e-362b42202b51/a7a19350-141d-48c0-b586-253cb3ec5b18/CTPcdg_ch_2016-10-27.pdf) |
| S20 | SEC Knight Capital Americas LLC 行政命令。 | market_and_protocol/PROJECT03_M0_V1.0.md:548 | 监管机关官方原文 | 本轮抽查官方正文可达并核对本包使用的事故要点；非取得内部原始数据 | Release No. 70694，2013-10-16，File No. 3-15570；已读事故事实相关段落。适用美国 2012 年历史事件，本项目为机制迁移实验。[SEC 原文 PDF](https://www.sec.gov/litigation/admin/2013/34-70694.pdf) |
| S21 | Nasdaq 对 2013 年 8 月 22 日事件的更新。 | market_and_protocol/PROJECT03_M0_V1.0.md:550 | 事故相关机构官方初步更新 | 本轮抽查官方正文可达并核对本包使用的事故要点；非取得内部原始数据 | 2013-08-29 官方初步调查更新；已读相关结论。早期通报与后续更新应按日期区分，不将 HFT 误当已确认根因。[Nasdaq 官方更新](https://ir.nasdaq.com/news-releases/news-release-details/nasdaq-omx-provides-updates-events-august-22-2013) |
| S22 | 证监会光大证券异常交易事件调查处理情况。 | market_and_protocol/PROJECT03_M0_V1.0.md:552 | 监管机关官方原文 | 本轮抽查官方正文可达并核对本包使用的事故要点；非取得内部原始数据 | 2013-08-30；已读技术过程与额度控制事实。文中亦有后续处置信息，本报告仅提取工程实验需要的技术背景。[监管原文](https://www.csrc.gov.cn/csrc/c100028/c1002255/content.shtml) |
| S23 | 上交所 2024 年股票竞价交易异常公告。 | market_and_protocol/PROJECT03_M0_V1.0.md:554 | 交易所官方发布/目录（目录不等于完整附件审查） | 本轮抽查官方正文可达并核对本包使用的事故要点；非取得内部原始数据 | 上证公告〔2024〕31 号及 32 号，2024-09-27；已核验异常现象与恢复时刻，内部根因未确认。[31 号公告](https://www.sse.com.cn/disclosure/announcement/general/c/c_20240927_10762480.shtml)；[32 号公告](https://www.sse.com.cn/disclosure/announcement/general/c/c_20240927_10762482.shtml) |
| S24 | 上期所 NGES 交易和行情接口规范。 | market_and_protocol/PROJECT03_M0_V1.0.md:556 | 交易所官方发布/目录（目录不等于完整附件审查） | 沿用原报告核验记录；本轮未逐条复验，见原限定 | v2.00，2025-09-12 发布，已核验 PDF 版本与官方入口；2026 年 7 月另有程序包目录。仅用于接口层级对照，未声称完成全规范审查或实接。[规范发布页](https://www.shfe.com.cn/services/technology/technical_download/202509/t20250912_829016.html)；[官方更新目录](https://www.shfe.com.cn/services/technology/technical_download/) |
| S25 | HdrHistogram 作者仓库。 | market_and_protocol/PROJECT03_M0_V1.0.md:558 | 作者/维护者一手方法资料；非交易规范 | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 在线方法与实现说明，未冻结项目依赖版本。用于直方图与线程隔离方法参考，不能照搬 Java 版的具体性能为 C++ 项目结果。[作者资料](https://github.com/HdrHistogram/HdrHistogram) |
| S26 | 上期所投教网 SimNow 使用指南。 | market_and_protocol/PROJECT03_M0_V1.0.md:560 | 交易所官方发布/目录（目录不等于完整附件审查） | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 动态页面，无独立版本号；已核验平台归属及注册路径，当前注册与账号能力仍须实际确认。[官方指南](https://edu.shfe.com.cn/home/simulate/simnow.html) |
| S27 | wrk2 作者仓库。 | market_and_protocol/PROJECT03_M0_V1.0.md:562 | 作者/维护者一手方法资料；非交易规范 | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 在线 README，无本项目依赖冻结；已读恒定负载与 Coordinated Omission 的方法说明，仅引用测量思想。[作者资料](https://github.com/giltene/wrk2) |
| S28 | Linux clock_gettime 手册。 | market_and_protocol/PROJECT03_M0_V1.0.md:564 | Linux手册技术一手参考；非柜台协议 | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 在线 man-pages；已核验时钟语义，具体运行内核与时钟读取开销待实测。[原始手册](https://man7.org/linux/man-pages/man2/clock_gettime.2.html) |
| S29 | 中金所交易员应用程序接口官方存档。 | market_and_protocol/PROJECT03_M0_V1.0.md:566 | 交易所官方发布/目录（目录不等于完整附件审查） | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 官方文件路径为 2018 年 12 月，检索正文含报单接口；完整附件抓取受限，精确规范版本与当前适用范围待确认。只证明候选资料线索，未据此冻结金融期货接口。[官方存档](https://www.cffex.com.cn/u/cms/www/201812/06163529dh4x.pdf) |
| R01 | 上期所螺纹钢期货业务细则。 | architecture/PROJECT03_系统架构设计_V1.1.md:873 | 交易所官方发布/目录（目录不等于完整附件审查） | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 2026-01-01 发布并施行；支持 RB 合约单位、价格步长等规则。临时参数与具体合约另行核验。[官方正文](https://www.shfe.com.cn/regulation/exchangerules/productrules/202512/t20251231_829962.html) |
| R02 | 上期所交易规则。 | architecture/PROJECT03_系统架构设计_V1.1.md:875 | 交易所官方发布/目录（目录不等于完整附件审查） | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 官网发布日期 2026-06-12；用于交易日、当日有效和交易所参与边界，日历与临时公告仍须按运行日期加载。[官方正文](https://www.shfe.cn/regulation/exchangerules/rules/202606/t20260603_831934.html) |
| R03 | 上期技术 CTP 客户端开发指南。 | architecture/PROJECT03_系统架构设计_V1.1.md:877 | 厂商原著、期货公司官网托管；历史指南 | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 厂商编写、东方财富期货官网托管；正文修订表 V3.1、2015-10-16，文件名含 2016-10-27。已核对基本请求、订单与成交回报、今昨仓相关内容；仅作历史语义线索，不视作当前 SDK 规范。[厂商指南 PDF](https://www.eastmoneyfutures.com/software/9809db08-c8cb-4cc4-b631-8a16f0c2dfb9/0f757a06-10a5-48c2-817e-362b42202b51/a7a19350-141d-48c0-b586-253cb3ec5b18/CTPcdg_ch_2016-10-27.pdf) |
| R04 | 中信期货信息系统外部接入说明。 | architecture/PROJECT03_系统架构设计_V1.1.md:879 | 期货公司官方说明；不是交易所统一协议 | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 期货公司官方动态页面，确认 API 与评测入口及接入审核背景；当前可达性、账户、SDK 模式和功能未实测。[官方页面](https://www.citicf.com/e-futures/csc/app/external_access) |
| R05 | 上期所第二代行情发布平台接口规范 V1.22。 | architecture/PROJECT03_系统架构设计_V1.1.md:881 | 交易所官方发布/目录（目录不等于完整附件审查） | 本轮核对V1.22官方发布目录；PDF抓取超时，未重新逐字段审查/保存原PDF | 文档日期 2026-07-21，官网发布页日期 2026-07-28；重点约束 MBP 切片、MDQP/MIRP、报文与快照编号、心跳和恢复。核验沿用 M0 与 V1.0 的相关章节及官方入口记录；V1.1 未新增协议能力结论。未声称已实现或获实时授权。[官方 PDF](https://www.shfe.com.cn/services/technology/technical_download/202607/P020260728318635042109.pdf) |
| R06 | SEC Knight Capital Americas LLC 行政命令。 | architecture/PROJECT03_系统架构设计_V1.1.md:883 | 监管机关官方原文 | 本轮抽查官方正文可达并核对本包使用的事故要点；非取得内部原始数据 | Release No. 70694，2013-10-16；历史背景沿用 M0 已核验事实，不作原系统复刻声明。[SEC PDF](https://www.sec.gov/litigation/admin/2013/34-70694.pdf) |
| R07 | Nasdaq 对 2013 年 8 月 22 日事件的更新。 | architecture/PROJECT03_系统架构设计_V1.1.md:885 | 事故相关机构官方初步更新 | 本轮抽查官方正文可达并核对本包使用的事故要点；非取得内部原始数据 | 2013-08-29 官方初步调查更新；用于连接与过载机制背景，不将模拟缺口当作历史原始流量。[官方更新](https://ir.nasdaq.com/news-releases/news-release-details/nasdaq-omx-provides-updates-events-august-22-2013) |
| R08 | 证监会光大证券异常交易调查处理情况。 | architecture/PROJECT03_系统架构设计_V1.1.md:887 | 监管机关官方原文 | 本轮抽查官方正文可达并核对本包使用的事故要点；非取得内部原始数据 | 2013-08-30；用于错误订单与在途风险控制背景，案例范围沿用 M0。[监管原文](https://www.csrc.gov.cn/csrc/c100028/c1002255/content.shtml) |
| R09 | 上交所 2024 年股票竞价交易异常公告。 | architecture/PROJECT03_系统架构设计_V1.1.md:889 | 交易所官方发布/目录（目录不等于完整附件审查） | 本轮抽查官方正文可达并核对本包使用的事故要点；非取得内部原始数据 | 上证公告〔2024〕31、32 号，2024-09-27；只支持外部确认缓慢及恢复现象，未确认内部根因。[31 号](https://www.sse.com.cn/disclosure/announcement/general/c/c_20240927_10762480.shtml)；[32 号](https://www.sse.com.cn/disclosure/announcement/general/c/c_20240927_10762482.shtml) |
| R10 | 项目立项与中国市场技术选型报告 V1.0。 | architecture/PROJECT03_系统架构设计_V1.1.md:891 | 项目内部报告/历史入口，按原件说明 | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 项目 M0 报告，V1.0 编制时已读取；V1.1 按本次用户审查意见保留已批准选型；本报告沿用其市场、接口和技术诚信边界，不把 M0 设计当成已实现事实。[M0 报告](https://chatgpt.com/library?file_id=libfile_383917fadc64819190987118c741aeec&account_id=personal) |
| R11 | Linux man-pages write 2。 | architecture/PROJECT03_系统架构设计_V1.1.md:899 | Linux手册技术一手参考；非柜台协议 | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 当前在线 man-pages 6.19 页面，本版核对日期 2026-11-05；用于短写、错误与 write 成功不等于持久化的边界。适用 Linux 用户态文件接口，不提供柜台交易原子性。[write 文档](https://man7.org/linux/man-pages/man2/write.2.html) |
| R12 | Linux man-pages fsync 2。 | architecture/PROJECT03_系统架构设计_V1.1.md:901 | Linux手册技术一手参考；非柜台协议 | 沿用原报告核验记录；本轮未逐条复验，见原限定 | man-pages 6.19，页面日期 2026-02-08；本版核对成功返回、错误和目录同步边界。实际存储掉电保证仍需目标环境测试。[fsync 文档](https://man7.org/linux/man-pages/man2/fsync.2.html) |
| R13 | C++ 工作草案 mutex requirements。 | architecture/PROJECT03_系统架构设计_V1.1.md:903 | C++在线工作草案；非冻结ISO C++20文本 | 沿用原报告核验记录；本轮未逐条复验，见原限定 | 本版于 2026-11-05 查阅在线工作草案的 thread.mutex.requirements.mutex。用于同一 mutex 的顺序和同步关系；项目仍以 C++20 编译，在线草案不冒充固定 SDK 或已冻结的 ISO C++20 文本。[互斥量规范章节](https://eel.is/c++draft/thread.mutex.requirements.mutex) |

## 本次迁移的证据边界

原件来自已取得的17份项目附件，文件身份、原始文件名、版本与SHA256见ORIGINALS_PROVENANCE.json。正文/目录抽查不等于已取得官方PDF原始字节；当前SMDP PDF、SDK包和完整原生聊天导出缺失见MISSING_MATERIALS.md。
本次未执行交易核心、模型编译、外部连接、压力、事故或性能实验；只有交接文件/原始记录一致性自检。历史38/85结果有单独日志与指纹，不能将来源核对计入这些测试数量。
