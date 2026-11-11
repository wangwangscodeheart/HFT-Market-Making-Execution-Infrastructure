#!/usr/bin/env python3
"""Reproduce M1A build/test evidence. Standard library only; invokes no shell.

Usage: python3 scripts/verify.py --cmake /path/to/cmake [--ninja /path/to/ninja]
Every command and exit status is recorded. A nonzero/unexpected exit aborts.
"""
import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import platform
import shlex
import shutil
import subprocess
import sys

ROOT = Path(__file__).resolve().parents[1]

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--cmake", default="cmake")
    parser.add_argument("--ninja")
    parser.add_argument("--jobs", default="2")
    parser.add_argument("--evidence", default="evidence")
    parser.add_argument("--profiles", nargs="+", choices=("debug","release","sanitizers"), default=["debug","release","sanitizers"])
    parser.add_argument("--detect-leaks", choices=("0","1"), default="0",
                        help="Enable LSan only on a host allowing its /proc/task inspection")
    args = parser.parse_args()
    evidence = ROOT / args.evidence
    evidence.mkdir(parents=True, exist_ok=True)
    command_log = evidence / "commands.json"
    records = json.loads(command_log.read_text()) if command_log.exists() else []
    env = dict(os.environ, ASAN_OPTIONS=f"detect_leaks={args.detect_leaks}:halt_on_error=1",
               UBSAN_OPTIONS="halt_on_error=1:print_stacktrace=1")
    def run(name, command, expected=0):
        result = subprocess.run([str(x) for x in command], cwd=ROOT, env=env,
                                stdout=subprocess.PIPE, stderr=subprocess.STDOUT, text=True)
        (evidence / f"{name}.log").write_text(result.stdout, encoding="utf-8")
        records.append({"name": name, "command": [str(x) for x in command],
                        "expected_exit": expected, "exit": result.returncode})
        (evidence / "commands.json").write_text(json.dumps(records, indent=2), encoding="utf-8")
        print(f"{name}: exit={result.returncode} expected={expected}", flush=True)
        if result.returncode != expected:
            print(result.stdout[-12000:])
            raise SystemExit(1)
        return result.stdout
    cmake = Path(args.cmake).resolve() if "/" in args.cmake else args.cmake
    ctest = Path(cmake).with_name("ctest") if "/" in str(cmake) else "ctest"
    run("cmake-version", [cmake,"--version"])
    run("compiler-version", [os.environ.get("CXX","c++"),"--version"])
    run("cpu", ["lscpu"])
    (evidence / "environment.json").write_text(json.dumps({
        "recorded_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "platform": platform.platform(), "machine": platform.machine(), "python": sys.version,
        "cwd": str(ROOT), "asan_options": env["ASAN_OPTIONS"],
        "ubsan_options": env["UBSAN_OPTIONS"], "performance_measurement": False,
    }, ensure_ascii=False, indent=2), encoding="utf-8")
    for profile in args.profiles:
        build = ROOT / "build" / profile
        configure = [cmake,"-S",ROOT,"-B",build,"-DCMAKE_BUILD_TYPE=" + ("Release" if profile=="release" else "Debug"),
                     "-DBUILD_TESTING=ON", "-DP03_SANITIZE=" + ("ON" if profile=="sanitizers" else "OFF")]
        if args.ninja:
            configure += ["-G","Ninja","-DCMAKE_MAKE_PROGRAM="+str(Path(args.ninja).resolve())]
        run(profile+"-configure",configure)
        run(profile+"-build",[cmake,"--build",build,"--parallel",args.jobs])
        shutil.copy2(build/"compile_commands.json",evidence/(profile+"-compile-commands.json"))
        run(profile+"-test-list",[build/"p03_tests","--list"])
        run(profile+"-tests",[build/"p03_tests"])
        run(profile+"-ctest",[ctest,"--test-dir",build,"--output-on-failure","--output-junit",evidence/(profile+"-junit.xml")])
        run(profile+"-demo",[build/"p03_m1a_demo"])
    run("test-runner-negative-control",[ROOT/"build/release/p03_tests","--self-check-failure"],expected=1)
    run("unknown-test-negative-control",[ROOT/"build/release/p03_tests","--case","does_not_exist"],expected=2)
    inventory = {}
    for path in sorted(ROOT.rglob("*")):
        if not path.is_file() or any(x in path.relative_to(ROOT).parts for x in ("build",".git","evidence","__pycache__")):
            continue
        inventory[str(path.relative_to(ROOT))] = hashlib.sha256(path.read_bytes()).hexdigest()
    (evidence/"source-sha256.json").write_text(json.dumps(inventory,indent=2,ensure_ascii=False),encoding="utf-8")
    print("Verification completed; inspect commands.json and all profile logs.")

if __name__ == "__main__":
    main()
