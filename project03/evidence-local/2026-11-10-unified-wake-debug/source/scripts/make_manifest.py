#!/usr/bin/env python3
"""Cross-check actual named tests, CTest JUnit and source declarations; write delivery manifests."""
import hashlib
import json
from pathlib import Path
import re
import xml.etree.ElementTree as ET

ROOT = Path(__file__).resolve().parents[1]
EVIDENCE = ROOT / "evidence"

def main():
    declared = {}
    for path in sorted((ROOT/"tests").glob("*_tests.cpp")):
        for line_no,line in enumerate(path.read_text().splitlines(),1):
            match = re.match(r"TEST_CASE\((\w+)\)",line)
            if match:
                name = match[1]
                if name in declared:
                    raise RuntimeError("duplicate test declaration: " + name)
                declared[name] = f"{path.relative_to(ROOT)}:{line_no}"
    summary = {"scope":"M1A business implementation only", "profiles":{}, "performance_measurement":False,
               "lsan":"Blocked: task scanning did not complete; detect_leaks=0 in final sanitizer run"}
    for profile in ("debug","release","sanitizers"):
        listed = (EVIDENCE/f"{profile}-test-list.log").read_text().splitlines()
        if len(listed)!=len(set(listed)) or set(listed)!=set(declared):
            raise RuntimeError(profile+" test list differs from source declarations")
        junit = ET.parse(EVIDENCE/f"{profile}-junit.xml").getroot()
        cases = list(junit.iter("testcase"))
        if {c.attrib["name"] for c in cases}!=set(declared) or len(cases)!=len(declared):
            raise RuntimeError(profile+" CTest coverage differs from source")
        if any(c.find("failure") is not None or c.find("error") is not None or c.find("skipped") is not None for c in cases):
            raise RuntimeError(profile+" JUnit has failures/skips")
        output = (EVIDENCE/f"{profile}-tests.log").read_text()
        match = re.search(r"RESULT cases=(\d+) failed=(\d+) assertions=(\d+)",output)
        if not match or int(match[1])!=len(declared) or int(match[2])!=0:
            raise RuntimeError(profile+" aggregate run did not pass")
        summary["profiles"][profile] = {"tests":int(match[1]),"failed":0,"assertions":int(match[3]),
                                         "ctest_names_match_source":True}
    lines = ["# M1A 自动化测试清单", "", "本清单由源码声明、已运行的测试清单和 CTest JUnit 三方核对生成。",
             "每例在 Debug、Release、ASan/UBSan 下均实际通过；没有 skip。LSan 未通过验收。",
             "完整逐例检查次数见 evidence/*-tests.log；每种构建均包含 O02 的 720 种排列。", "",
             "| 测试 | 架构契约/范围 | 源文件位置 | 最终三构建结果 |", "|---|---|---|---|"]
    for name in sorted(declared):
        ids = sorted(set(re.findall(r"(?:^|_)([AOP]\d{2})(?=_|$)",name)))
        lines.append(f"| `{name}` | {'、'.join(ids) or 'M1A 基础/边界'} | `{declared[name]}` | PASS / PASS / PASS |")
    lines += ["", "## 外部与后续能力", "", "J/K/Q 契约、真实 CTP/SimNow、发送、日志文件、恢复资格和生产回报队列不在这张通过清单中。",
              "测试请求它们时只验证返回 NotImplemented/Blocked，不能据此宣称相应模块实现通过。", ""]
    (ROOT/"docs/M1A_TEST_MATRIX.md").write_text("\n".join(lines),encoding="utf-8")
    (EVIDENCE/"acceptance-summary.json").write_text(json.dumps(summary,indent=2,ensure_ascii=False),encoding="utf-8")
    inventory = {}
    for path in sorted(ROOT.rglob("*")):
        if not path.is_file() or any(p in ("build",".git","evidence","__pycache__") for p in path.relative_to(ROOT).parts):
            continue
        inventory[str(path.relative_to(ROOT))] = hashlib.sha256(path.read_bytes()).hexdigest()
    (EVIDENCE/"source-sha256.json").write_text(json.dumps(inventory,indent=2,ensure_ascii=False),encoding="utf-8")
    print(json.dumps(summary,ensure_ascii=False))

if __name__ == "__main__":
    main()
