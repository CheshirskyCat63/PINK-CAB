#!/usr/bin/env python3
"""Materialize required Git LFS fixtures from verified local mirrors.

This helper is for self-hosted PINK CAB CI when GitHub LFS bandwidth is
unavailable. It never substitutes an unverified asset: every recovered file
must hash exactly to the SHA-256 OID recorded in the repository's LFS pointer.
"""

from __future__ import annotations

import argparse
import hashlib
import os
from pathlib import Path
import shutil
import subprocess
import sys
from typing import Iterable


POINTER_VERSION = "version https://git-lfs.github.com/spec/v1"


def parse_lfs_pointer(path: Path) -> str | None:
    try:
        raw = path.read_text(encoding="utf-8")
    except (UnicodeDecodeError, OSError):
        return None
    lines = raw.splitlines()
    if not lines or lines[0].strip() != POINTER_VERSION:
        return None
    for line in lines:
        if line.startswith("oid sha256:"):
            oid = line.split(":", 1)[1].strip().lower()
            if len(oid) == 64 and all(ch in "0123456789abcdef" for ch in oid):
                return oid
    raise RuntimeError(f"Malformed LFS pointer: {path}")


def sha256_file(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as stream:
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            digest.update(chunk)
    return digest.hexdigest()


def iter_named_candidates(
    roots: Iterable[Path],
    filename: str,
    workspace: Path,
) -> Iterable[Path]:
    workspace = workspace.resolve()
    for root in roots:
        if not root.exists():
            print(f"PINKCAB_LOCAL_LFS_SEARCH_ROOT_MISSING={root}")
            continue
        print(f"PINKCAB_LOCAL_LFS_SEARCH_ROOT={root}")
        for current, dirs, files in os.walk(root):
            current_path = Path(current)
            # Avoid scanning the active worktree recursively and avoid expensive
            # transient/intermediate trees that cannot be authoritative sources.
            try:
                resolved = current_path.resolve()
                if resolved == workspace or workspace in resolved.parents:
                    dirs[:] = []
                    continue
            except OSError:
                pass
            dirs[:] = [
                d for d in dirs
                if d not in {".git", "Intermediate", "Saved", "DerivedDataCache"}
            ]
            if filename in files:
                yield current_path / filename


def lfs_object_path(repo: Path, oid: str) -> Path:
    return repo / ".git" / "lfs" / "objects" / oid[:2] / oid[2:4] / oid


def materialize_required(
    workspace: Path,
    relative: Path,
    search_roots: list[Path],
) -> None:
    target = workspace / relative
    oid = parse_lfs_pointer(target) if target.exists() else None
    if oid is None:
        if target.exists():
            print(f"PINKCAB_LOCAL_LFS_ALREADY_MATERIALIZED={relative}")
            return
        raise RuntimeError(f"Required fixture missing and no pointer exists: {relative}")

    cached = lfs_object_path(workspace, oid)
    if cached.exists() and sha256_file(cached) == oid:
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(cached, target)
        print(f"PINKCAB_LOCAL_LFS_FROM_OBJECT_CACHE={relative} oid={oid}")
        return

    matches: list[Path] = []
    for candidate in iter_named_candidates(search_roots, target.name, workspace):
        if candidate.resolve() == target.resolve():
            continue
        if parse_lfs_pointer(candidate) is not None:
            continue
        try:
            actual = sha256_file(candidate)
        except OSError:
            continue
        print(
            f"PINKCAB_LOCAL_LFS_CANDIDATE={candidate} "
            f"sha256={actual} expected={oid}"
        )
        if actual == oid:
            matches.append(candidate)

    if not matches:
        raise RuntimeError(
            f"No byte-identical local source for {relative}; expected sha256={oid}"
        )

    source = matches[0]
    cached.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(source, cached)
    if sha256_file(cached) != oid:
        raise RuntimeError(f"Local LFS cache verification failed for {relative}")

    target.parent.mkdir(parents=True, exist_ok=True)
    shutil.copy2(cached, target)
    if sha256_file(target) != oid:
        raise RuntimeError(f"Materialized fixture verification failed for {relative}")

    print(
        f"PINKCAB_LOCAL_LFS_RECOVERED={relative} "
        f"source={source} oid={oid} matches={len(matches)}"
    )


def git_lines(workspace: Path, *args: str) -> list[str]:
    proc = subprocess.run(
        ["git", *args],
        cwd=workspace,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if proc.returncode != 0:
        raise RuntimeError(
            f"git {' '.join(args)} failed ({proc.returncode}): {proc.stderr.strip()}"
        )
    return [line.strip() for line in proc.stdout.splitlines() if line.strip()]


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--workspace", type=Path, default=Path.cwd())
    parser.add_argument("--base-ref", default="origin/main")
    parser.add_argument("--required", action="append", default=[])
    parser.add_argument("--search-root", action="append", default=[])
    args = parser.parse_args()

    workspace = args.workspace.resolve()
    required = [Path(value) for value in args.required]
    roots = [Path(value) for value in args.search_root]

    subprocess.run(
        ["git", "lfs", "install", "--local"],
        cwd=workspace,
        check=True,
    )
    subprocess.run(
        ["git", "lfs", "checkout"],
        cwd=workspace,
        check=True,
    )

    tracked = set(git_lines(workspace, "lfs", "ls-files", "-n"))
    changed = git_lines(workspace, "diff", "--name-only", f"{args.base_ref}...HEAD")
    changed_lfs = sorted(set(changed) & tracked)
    if changed_lfs:
        for relative in changed_lfs:
            print(f"PINKCAB_LOCAL_LFS_CHANGED={relative}")
        raise RuntimeError(
            f"Code-only gate contains {len(changed_lfs)} changed LFS file(s)"
        )

    for relative in required:
        materialize_required(workspace, relative, roots)

    unresolved: list[str] = []
    for relative in sorted(tracked):
        path = workspace / relative
        if not path.exists() or parse_lfs_pointer(path) is not None:
            unresolved.append(relative)

    for relative in required:
        path = workspace / relative
        if not path.exists() or parse_lfs_pointer(path) is not None:
            raise RuntimeError(f"Required fixture remains unresolved: {relative}")

    print(
        "PINKCAB_LOCAL_LFS=PASS "
        f"required={len(required)} tracked={len(tracked)} "
        f"unresolved_unrelated={len(unresolved)}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"PINKCAB_LOCAL_LFS=FAIL {exc}", file=sys.stderr)
        raise
