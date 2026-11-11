#!/usr/bin/env python3
"""Verify handover files and historical evidence; never build/run trading code or use the network.

Default checks an existing inventory and writes integrity results only.
--build-manifest is for deliberately regenerating a documentation delivery after review.
"""
from pathlib import Path
import argparse
import hashlib
import json
import re
import sys
import urllib.parse
import xml.etree.ElementTree as ET
import zipfile

ROOT=Path(__file__).resolve().parents[1]
EXCLUDED={
    'manifest/FILE_MANIFEST.md','manifest/FILES.json','manifest/SHA256SUMS',
    'manifest/INTEGRITY_CHECK_RESULTS.md','manifest/INTEGRITY_CHECK_RESULTS.json'
}
ROOT_DOCS=[
    '00_START_HERE.md','01_PROJECT_CONTEXT.md','02_PROJECT_HISTORY.md','03_MARKET_AND_PROTOCOL.md',
    '04_ARCHITECTURE_BASELINE.md','05_ADR_REGISTER.md','06_P0_CONTRACTS.md','07_IMPLEMENTATION_STATUS.md',
    '08_M1A_EXECUTION_PLAN.md','09_TEST_AND_ACCEPTANCE.md','10_INCIDENT_LAB.md','11_WEB_WORKSTATION.md',
    '12_PERFORMANCE_AND_RELIABILITY.md','13_LEARNING_AND_MENTORSHIP.md','14_OPEN_QUESTIONS_AND_RISKS.md',
    '15_CODEX_WORKING_RULES.md'
]
def digest(path):return hashlib.sha256(path.read_bytes()).hexdigest()
def all_files():
    return [p for p in sorted(ROOT.rglob('*')) if p.is_file() and
            not any(x in ('__pycache__','build') for x in p.relative_to(ROOT).parts)]
def kind(relative):
    if '/originals/' in relative or relative.startswith('architecture/') or relative.startswith('market_and_protocol/'):
        return '原件/原件副本或其阅读说明（依来源登记区分）'
    if relative.startswith('implementation/m1a_'):return '原ZIP展开的候选代码/历史证据，不是本轮编码'
    if relative.startswith('reference_model/expanded/'):return '原参考模型ZIP展开'
    if relative.startswith('adr/'):return '依据已批准基线整理的ADR，非独立历史原件'
    if relative.startswith('manifest/'):return '交接清单/证据索引'
    return '本轮整理件/工具（不构成新架构）'

def make_inventory():
    entries=[]
    for p in all_files():
        name=p.relative_to(ROOT).as_posix()
        if name not in EXCLUDED:entries.append({'path':name,'bytes':p.stat().st_size,'sha256':digest(p),'kind':kind(name)})
    (ROOT/'manifest/FILES.json').write_text(json.dumps({'excluded_outputs':sorted(EXCLUDED),'files':entries},indent=2,ensure_ascii=False))
    (ROOT/'manifest/SHA256SUMS').write_text(''.join(f"{x['sha256']}  {x['path']}\n" for x in entries))
    originals=json.loads((ROOT/'manifest/ORIGINALS_PROVENANCE.json').read_text())
    lines=['# 文件清单与完整性边界','',
        '用户要求的M0 DOCX/MD、V1.1 DOCX/MD、变更清单和参考模型ZIP原件均已取得：必需原件缺失0。共保留17份来源原件，包含同内容副本和额外M1A候选交付。',
        '原件未重建、未改字节；每份来源身份和时间/版本见ORIGINALS_PROVENANCE.json。其余缺口见MISSING_MATERIALS.md。',
        '', '## 来源原件（17份）','', '| 原文件名 | 本包位置 | 字节 | SHA256 |','|---|---|---:|---|']
    for x in originals:lines.append(f"| {x['original_filename']} | `{x['path']}` | {x['size_bytes']} | `{x['sha256']}` |")
    lines+=['','## 全包文件','',
        '以下清单列出校验输入。为避免自引用哈希循环，FILE_MANIFEST.md、FILES.json、SHA256SUMS和两份INTEGRITY_CHECK_RESULTS输出不对自身计算哈希；它们真实存在并另查结构/非空。build/和__pycache__/不属于交付。',
        '', '| 文件 | 字节 | SHA256 | 性质 |','|---|---:|---|---|']
    for x in entries:lines.append(f"| `{x['path']}` | {x['bytes']} | `{x['sha256']}` | {x['kind']} |")
    for name in sorted(EXCLUDED):lines.append(f'| `{name}` | 输出文件 | 自引用输出排除 | 清单/自检结果，不冒充原件 |')
    lines+=['','## 版本选择与缺失资料','',
        'architecture/根目录V1.1为正式技术基线；history/V1.0仅作历史。所有(1)副本与对应原件SHA相同，保留接收原名，绝非另一个生效版本。',
        '原报告中的待审状态由用户后续批准覆盖。当前已有M1A候选和历史85项记录，但用户未正式验收，不得改成“无实现”或“已验收”。',
        '必需项目报告和模型附件完整；全量原生聊天、用户独立复跑原始日志、当前SDK、外部联调、规范原PDF字节和未产生实验结果的缺口在MISSING_MATERIALS.md逐项列明。','']
    (ROOT/'manifest/FILE_MANIFEST.md').write_text('\n'.join(lines))

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--build-manifest',action='store_true')
    args=parser.parse_args()
    if args.build_manifest:make_inventory()
    checks=[]
    def check(name,condition,details):checks.append({'name':name,'passed':bool(condition),'details':details})
    files=all_files()
    check('required_16_documents',all((ROOT/x).is_file() and (ROOT/x).stat().st_size>0 for x in ROOT_DOCS),'00～15全部真实存在且非空')
    check('all_files_nonempty',all(p.stat().st_size>0 for p in files),'不以空文件冒充附件；合法文件逐个检查')
    provenance=json.loads((ROOT/'manifest/ORIGINALS_PROVENANCE.json').read_text())
    check('17_original_bytes',len(provenance)==17 and all((ROOT/x['path']).is_file() and (ROOT/x['path']).stat().st_size==x['size_bytes'] and digest(ROOT/x['path'])==x['sha256'] for x in provenance),'17份来源原件大小/SHA与取得时记录一致')
    required_originals=['market_and_protocol/PROJECT03_M0_V1.0.md','market_and_protocol/项目立项与中国市场技术选型报告_V1.0.docx','architecture/PROJECT03_系统架构设计_V1.1.md','architecture/PROJECT03_系统架构设计_V1.1.docx','architecture/PROJECT03_V1.1_变更清单.md','reference_model/originals/PROJECT03_V1.1_架构契约测试.zip']
    check('mandatory_originals_present',all((ROOT/x).is_file() for x in required_originals),'必需原件缺失0；模型ZIP内含源码/输出/元数据')
    archive_errors=[]
    for p in files:
        if p.suffix.lower() in ('.zip','.docx'):
            try:
                with zipfile.ZipFile(p) as z:
                    if z.testzip():archive_errors.append(str(p.relative_to(ROOT))+': CRC')
                    if any(Path(n).is_absolute() or '..' in Path(n).parts for n in z.namelist()):archive_errors.append(str(p.relative_to(ROOT))+': unsafe member')
                    if p.suffix=='.docx':ET.fromstring(z.read('word/document.xml'))
            except Exception as exc:archive_errors.append(str(p.relative_to(ROOT))+': '+str(exc))
    check('archives_docx_crc_and_structure',not archive_errors,archive_errors or '所有ZIP/DOCX CRC及DOCX正文XML有效；不是视觉等价审查')
    adrs=[]
    for i in range(1,11):
        p=ROOT/f'adr/ADR{i:03}.md'
        text=p.read_text() if p.exists() else ''
        adrs.append(all(s in text for s in ('决策背景','最终决定','原因','约束','当前状态','对应章节','Approved')))
    check('adr001_to_010',all(adrs),'十项背景/决定/原因/约束/状态/出处齐全，均标整理件而非历史原件')
    base=(ROOT/'architecture/PROJECT03_系统架构设计_V1.1.md').read_text()
    full=(ROOT/'04_ARCHITECTURE_BASELINE.md').read_text()
    check('full_v11_in_04',base.strip() in full,'04包含完整原Markdown正文，不只摘要；正式原件SHA另独立核对')
    p0=(ROOT/'06_P0_CONTRACTS.md').read_text()
    spans=[('### 12.1 连续提交前缀与确认','### 12.5 安全关闭与线程退出'),('### 5.3 发送与 Kill 的精确边界','## 6 事件 数据与时间模型'),('### 7.3 累计成交证据与数量分区','### 8.2 净仓与总仓同时检查'),('### 9.1 分桶更新规则','## 10 行情状态与独立协议实验'),('### 12.6 关键交易回报队列溢出后的资格','## 13 故障注入架构与事故事实边界'),('### 8.4 Kill 与持久化故障下的撤单','## 9 期货今昨仓与成交账本')]
    complete=[]
    for start,end in spans:
        begin=base.index(start);finish=base.index(end,begin+len(start))
        complete.append(base[begin:finish].strip() in p0)
    check('five_p0_and_adr008_complete',all(complete),'六段规范全文逐段与原件一致，含不变量、路径、反例、测试和阶段影响')
    begin=base.index('### 17.2 M1 分步交付');finish=base.index('### 17.3 后续阶段',begin)
    plan=(ROOT/'08_M1A_EXECUTION_PLAN.md').read_text()
    check('m1a_plan_matches_17_2',base[begin:finish].strip() in plan and all(s in plan for s in ('C++20','StepHarness','真实线程','持久化发送','已有未验收候选')),'原17.2完整保留；候选优先审查，禁止提前后续能力')
    model=ROOT/'reference_model/expanded/PROJECT03_V1.1_contracts'
    metadata=json.loads((model/'execution_metadata.json').read_text())
    code=(model/'contract_model_tests.cpp').read_text();output=(model/'model_results.txt').read_text()
    named=re.findall(r'test\("([JKOPQ]\d\d_[^\"]+)"',code)
    passed=re.findall(r'^PASS (\S+)',output,re.M)
    expected={f'{family}{i:02}' for family,n in [('J',7),('K',7),('O',9),('P',6),('Q',9)] for i in range(1,n+1)}
    check('model_38_names_and_hashes',len(named)==len(passed)==38 and set(named)==set(passed) and {x[:3] for x in named}==expected and digest(model/'contract_model_tests.cpp')==metadata['source_sha256'] and digest(model/'model_results.txt')==metadata['output_sha256'] and 'kill_schedules=5 oms_permutations=720' in output,'原始38名字/源码/输出哈希/5位置/720排列一致；没有重跑')
    candidate=ROOT/'implementation/m1a_candidate/project03'
    fingerprints=json.loads((candidate/'evidence/source-sha256.json').read_text())
    check('candidate_39_source_fingerprints',len(fingerprints)==39 and all(digest(candidate/n)==d for n,d in fingerprints.items()),'候选39项指纹全部匹配；本轮未改业务源码/历史文档')
    declarations=[]
    for p in (candidate/'tests').glob('*_tests.cpp'):declarations+=re.findall(r'^TEST_CASE\((\w+)\)',p.read_text(),re.M)
    test_checks=[]
    for profile in ('debug','release','sanitizers','clean'):
        text=(candidate/f'evidence/{profile}-tests.log').read_text()
        match=re.search(r'RESULT cases=(\d+) failed=(\d+) assertions=(\d+)',text)
        cases=list(ET.parse(candidate/f'evidence/{profile}-junit.xml').getroot().iter('testcase'))
        test_checks.append(match and match.groups()==('85','0','24868') and len(cases)==85 and {c.attrib['name'] for c in cases}==set(declarations) and all(c.find('failure') is None and c.find('error') is None and c.find('skipped') is None for c in cases))
    check('candidate_85_historical_records',len(declarations)==len(set(declarations))==85 and all(test_checks),'四组历史85例/JUnit/24868检查与源码一致；不代表本轮重跑或用户验收')
    distinct=ROOT/'implementation/m1a_evidence/evidence'
    check('candidate_separate_evidence_identical',all((candidate/'evidence'/p.name).read_bytes()==p.read_bytes() for p in distinct.iterdir() if p.is_file()),'独立evidence ZIP与source ZIP中的同名历史证据字节一致')
    start=(ROOT/'00_START_HERE.md').read_text();status=(ROOT/'07_IMPLEMENTATION_STATUS.md').read_text()
    check('current_status_disambiguated','当前状态：M0 完成，V1.1 架构正式批准，允许启动 M1A。' in start and '尚无用户正式验收通过' in start and '候选' in status and '待审查' in status, '批准时刻与当前候选分层说明；没有以旧模板抹去实现或自行验收')
    index=json.loads((ROOT/'manifest/REFERENCE_INDEX.json').read_text())
    expected_refs={f'S{i:02}' for i in range(1,30)}|{f'R{i:02}' for i in range(1,14)}
    check('42_reference_entries',len(index)==42 and {x['id'] for x in index}==expected_refs,'原S01～S29/R01～R13全部保留；来源层级/版本/限制明确')
    missing=(ROOT/'manifest/MISSING_MATERIALS.md').read_text()
    check('missing_materials_explicit',all(s in missing for s in ('必需原件缺失0','全量原生聊天','SMDP','SDK','用户独立机器重跑')),'必需原件与外部/历史资料缺口分开，未用空文件冒充')
    broken=[]
    for p in files:
        if p.suffix!='.md':continue
        # Markdown link checks only; ignore fenced code and immutable original explicit URLs.
        text=re.sub(r'```.*?```','',p.read_text(),flags=re.S)
        for target in re.findall(r'(?<!!)\[[^\]]+\]\(([^)]+)\)',text):
            target=target.strip().split(' "',1)[0].strip('<>')
            if not target or re.match(r'^[A-Za-z][A-Za-z0-9+.-]*:',target) or target.startswith('#'):continue
            path=urllib.parse.unquote(target.split('#',1)[0])
            if path and not (p.parent/path).exists():broken.append(str(p.relative_to(ROOT))+': '+target)
    check('local_markdown_links',not broken,broken or '本地Markdown目标均存在；未把公网链接可达性冒充内容或接口实测')
    inventory=json.loads((ROOT/'manifest/FILES.json').read_text())
    entries=inventory['files'];listed={x['path'] for x in entries}
    current={p.relative_to(ROOT).as_posix() for p in all_files()}-EXCLUDED
    checksum_ok=listed==current and all((ROOT/x['path']).stat().st_size==x['bytes'] and digest(ROOT/x['path'])==x['sha256'] for x in entries)
    check('inventory_sha256_and_coverage',checksum_ok,{'checksummed_files':len(entries),'missing':sorted(current-listed),'stale':sorted(listed-current),'self_excluded_outputs':sorted(EXCLUDED)})
    passed_count=sum(x['passed'] for x in checks)
    result={'scope':'Handover/documentation integrity only; no C++ or external trading execution','checks':len(checks),'passed':passed_count,'failed':len(checks)-passed_count,'mandatory_original_missing':0,'originals':17,'warnings':['M1A candidate exists but has no formal user acceptance; approved scope remains M1A.','Immutable historical originals contain superseded pending-review status; current overlay is explicit.','Full native conversation export and external SDK/specification binary/connection evidence are not provided.','LSan remains unverified; historical business tests were not rerun by this checker.'],'results':checks}
    (ROOT/'manifest/INTEGRITY_CHECK_RESULTS.json').write_text(json.dumps(result,indent=2,ensure_ascii=False))
    lines=['# 交接包完整性检查结果','',f"检查项：{result['checks']}；通过：{result['passed']}；失败：{result['failed']}。必需项目原件缺失：0；保留来源原件：17。",'',
           '本次执行的是资料/证据一致性自检，不编译、不运行任何C++模型/业务程序，不做外部连接，不构成M1A用户验收。','',
           '| 检查 | 结果 | 具体边界 |','|---|---|---|']
    for x in checks:lines.append(f"| {x['name']} | {'PASS' if x['passed'] else 'FAIL'} | {str(x['details']).replace('|','/')} |")
    lines+=['','## 必须保留的状态与资料限制','']+[f'- {x}' for x in result['warnings']]+['',
            '语义复核：00～15覆盖项目背景、演化、市场、完整基线、ADR/P0、实际状态、阶段、测试、四事故、七页Web、性能可靠性、带教、未决和工作规则。此为人工索引审读+上述结构/字节检查，不宣称形式化证明LLM理解全部历史。',
            'DOCX按ZIP/正文XML及原件指纹验证；本轮没有修改DOCX，也没有逐页视觉排版或跨格式逐句等价验收。',
            '对旧原件里的待审/未实现文字，采用后续用户批准和当前证据的时间层次覆盖，原件字节不改。',
            '缺失资料详情见MISSING_MATERIALS.md。用户要求的项目报告/模型原件齐全；未提供的外部与原生对话资料不补造。','']
    (ROOT/'manifest/INTEGRITY_CHECK_RESULTS.md').write_text('\n'.join(lines))
    print(json.dumps({'checks':result['checks'],'passed':result['passed'],'failed':result['failed'],'originals':17,'mandatory_original_missing':0},ensure_ascii=False))
    for item in checks:
        if not item['passed']:print('FAIL',item['name'],item['details'])
    return 0 if result['failed']==0 else 1

if __name__=='__main__':raise SystemExit(main())
