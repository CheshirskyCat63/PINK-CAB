from __future__ import annotations

import argparse
import json
import re
from pathlib import Path

SCRIPT_SUFFIXES = {".py", ".ps1", ".sh", ".bat", ".cmd"}

FORBIDDEN_ACTIVE_SCRIPT_PATTERNS = {
    "personal_user_path": re.compile(r"[A-Za-z]:\\\\Users\\\\", re.IGNORECASE),
    "local_worktree_path": re.compile(r"(?:[\\\\/])\\.worktrees(?:[\\\\/])", re.IGNORECASE),
    "downloads_path": re.compile(r"[A-Za-z]:\\\\[^\\\\\r\n]*\\\\Downloads\\\\", re.IGNORECASE),
}

REQUIRED_COOK_LINES = {
    '+DirectoriesToAlwaysCook=(Path="/Game/Dev/Vehicles/Tatra613ArchiveV12Clean/Tatra613_V12_Wheel")',
    '+DirectoriesToAlwaysCook=(Path="/Game/Dev/Vehicles/Tatra613DesktopScene/Tatra613_ScenePreserved/StaticMeshes")',
}

FORBIDDEN_COOK_LINES = {
    '+DirectoriesToAlwaysCook=(Path="/Game/Dev/Vehicles/Tatra613Donor")',
    '+DirectoriesToAlwaysCook=(Path="/Game/Dev/Vehicles/Tatra613ArchiveV12Clean")',
    '+DirectoriesToAlwaysCook=(Path="/Game/Dev/Vehicles/Tatra613DesktopScene")',
}


def scan_repository(root: Path) -> list[dict[str, str]]:
    root = root.resolve()
    violations: list[dict[str, str]] = []

    scripts_root = root / "scripts"
    if scripts_root.exists():
        for path in sorted(scripts_root.rglob("*")):
            if not path.is_file() or path.suffix.lower() not in SCRIPT_SUFFIXES:
                continue
            relative_to_scripts = path.relative_to(scripts_root)
            if "archive" in relative_to_scripts.parts:
                continue
            text = path.read_text(encoding="utf-8-sig", errors="replace")
            for rule, pattern in FORBIDDEN_ACTIVE_SCRIPT_PATTERNS.items():
                if pattern.search(text):
                    violations.append({
                        "rule": rule,
                        "path": path.relative_to(root).as_posix(),
                        "detail": "machine-specific path is forbidden in active tooling",
                    })

    config_path = root / "Config" / "DefaultGame.ini"
    if not config_path.exists():
        violations.append({
            "rule": "missing_default_game",
            "path": "Config/DefaultGame.ini",
            "detail": "DefaultGame.ini is required for cook-surface validation",
        })
    else:
        lines = {
            line.strip()
            for line in config_path.read_text(
                encoding="utf-8-sig", errors="replace"
            ).splitlines()
        }
        for line in sorted(REQUIRED_COOK_LINES - lines):
            violations.append({
                "rule": "missing_required_cook",
                "path": "Config/DefaultGame.ini",
                "detail": line,
            })
        for line in sorted(FORBIDDEN_COOK_LINES & lines):
            violations.append({
                "rule": "forbidden_broad_cook",
                "path": "Config/DefaultGame.ini",
                "detail": line,
            })

    return violations


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--root", required=True)
    args = parser.parse_args()

    violations = scan_repository(Path(args.root))
    print(json.dumps({
        "repo_hygiene_violations": len(violations),
        "violations": violations,
    }, indent=2))

    return 1 if violations else 0


if __name__ == "__main__":
    raise SystemExit(main())
