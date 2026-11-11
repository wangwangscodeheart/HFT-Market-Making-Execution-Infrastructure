#!/usr/bin/env python3
"""Package source, docs and raw evidence; exclude all build artifacts and caches."""
import argparse
from pathlib import Path
import zipfile

ROOT = Path(__file__).resolve().parents[1]
def main():
    parser=argparse.ArgumentParser()
    parser.add_argument("--output",type=Path,default=ROOT.parent/"PROJECT03_M1A_source.zip")
    args=parser.parse_args()
    args.output.parent.mkdir(parents=True,exist_ok=True)
    files=[]
    for path in sorted(ROOT.rglob("*")):
        relative=path.relative_to(ROOT)
        if path.is_file() and not any(p in ("build",".git","__pycache__") for p in relative.parts):
            files.append((path,Path("project03")/relative))
    with zipfile.ZipFile(args.output,"w",compression=zipfile.ZIP_DEFLATED,compresslevel=9) as archive:
        for path,relative in files:
            info=zipfile.ZipInfo(relative.as_posix(),date_time=(2026,11,5,0,0,0))
            info.compress_type=zipfile.ZIP_DEFLATED
            info.external_attr=0o100644 << 16
            archive.writestr(info,path.read_bytes())
    print(f"Packaged {len(files)} files: {args.output}")
if __name__=="__main__":
    main()
