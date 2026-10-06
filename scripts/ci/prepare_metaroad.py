#!/usr/bin/env python3
"""Provision only the frozen free MetaRoad content used by the baked P4 road."""
from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import shutil

EXPECTED_VERSION = "3.2.0"

def read_version(root: Path) -> str:
    path = root / "MetaRoad.uplugin"
    if not path.is_file():
        raise ValueError(f"missing {path}")
    return str(json.loads(path.read_text(encoding="utf-8")).get("VersionName", ""))

def freeze_as_content_only(root: Path) -> None:
    descriptor = {
        "FileVersion": 3,
        "Version": 1,
        "VersionName": EXPECTED_VERSION,
        "FriendlyName": "PINK CAB Frozen Road Materials",
        "Description": "Frozen free MetaRoad content retained only for baked P4 road materials.",
        "Category": "Content",
        "CanContainContent": True,
        "Installed": False
    }
    (root / "MetaRoad.uplugin").write_text(
        json.dumps(descriptor, indent=4) + "\n", encoding="utf-8")
    for generated_or_code in ("Source", "Binaries", "Intermediate"):
        path = root / generated_or_code
        if path.exists():
            shutil.rmtree(path)

def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument("--source", type=Path, default=Path(os.environ.get(
        "PINKCAB_METAROAD_PACKAGE",
        r"E:\CHESHIRE_DIVISION\Shared\Vendor\MetaRoad\3.2.0-ox4rZYlcREy004nfGp7EHg")))
    args = parser.parse_args()
    target = args.root / "Plugins" / "MetaRoad"
    try:
        if not target.exists():
            if read_version(args.source) != EXPECTED_VERSION:
                raise ValueError(f"vendor version must be {EXPECTED_VERSION}")
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copytree(args.source, target)
        found = read_version(target)
        if found != EXPECTED_VERSION:
            raise ValueError(f"MetaRoad version {found!r} != {EXPECTED_VERSION!r}")
        freeze_as_content_only(target)
    except (OSError, ValueError, json.JSONDecodeError) as error:
        print(f"PINKCAB_ROAD_VENDOR=FAIL {error}")
        return 1
    print(f"PINKCAB_ROAD_VENDOR=PASS MetaRoad={EXPECTED_VERSION} mode=content-only")
    return 0

if __name__ == "__main__":
    raise SystemExit(main())
