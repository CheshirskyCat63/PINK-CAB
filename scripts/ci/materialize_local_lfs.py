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


def external_lfs_object_path(repo: Path, oid: str) -> Path | None:
    """Resolve an exact historical LFS object from another local Git mirror."""
    proc = subprocess.run(
        ["git", "rev-parse", "--git-common-dir"],
        cwd=repo,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if proc.returncode != 0:
        return None
    common = Path(proc.stdout.strip())
    if not common.is_absolute():
        common = (repo / common).resolve()
    return common / "lfs" / "objects" / oid[:2] / oid[2:4] / oid


def iter_nested_lfs_object_candidates(
    roots: Iterable[Path],
    oid: str,
    workspace: Path,
) -> Iterable[Path]:
    """Find an exact LFS object in Git caches nested below trusted roots.

    Self-hosted runners and recovery trees can contain multiple historical
    clones. Their worktrees may no longer hold the requested asset revision,
    while the Git LFS object cache still does. Only the exact OID path is
    considered and every yielded file is hash-verified by the caller.
    """
    workspace = workspace.resolve()
    seen: set[Path] = set()
    prune_names = {
        "Binaries",
        "Content",
        "DerivedDataCache",
        "Intermediate",
        "Saved",
        "Builds",
        "node_modules",
    }

    for root in roots:
        if not root.exists():
            print(f"PINKCAB_LOCAL_LFS_CACHE_ROOT_MISSING={root}")
            continue
        print(f"PINKCAB_LOCAL_LFS_CACHE_ROOT={root}")

        direct = external_lfs_object_path(root, oid)
        if direct is not None and direct.exists():
            resolved_direct = direct.resolve()
            if resolved_direct not in seen:
                seen.add(resolved_direct)
                yield direct

        for current, dirs, files in os.walk(root):
            current_path = Path(current)
            try:
                resolved = current_path.resolve()
                if resolved == workspace or workspace in resolved.parents:
                    dirs[:] = []
                    continue
            except OSError:
                pass

            if current_path.name in prune_names:
                dirs[:] = []
                continue

            # A normal clone has a .git directory; a linked worktree has a
            # .git file. In both cases ask Git for the real common directory.
            if ".git" in dirs or ".git" in files:
                candidate = external_lfs_object_path(current_path, oid)
                if candidate is not None and candidate.exists():
                    try:
                        resolved_candidate = candidate.resolve()
                    except OSError:
                        resolved_candidate = candidate
                    if resolved_candidate not in seen:
                        seen.add(resolved_candidate)
                        yield candidate
                if ".git" in dirs:
                    dirs.remove(".git")

            dirs[:] = [d for d in dirs if d not in prune_names]


def is_git_worktree(workspace: Path) -> bool:
    proc = subprocess.run(
        ["git", "rev-parse", "--is-inside-work-tree"],
        cwd=workspace,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    return proc.returncode == 0 and proc.stdout.strip().lower() == "true"


def materialize_cached_lfs_object(
    workspace: Path,
    relative: Path,
    oid: str,
) -> None:
    """Copy a verified LFS object into the worktree and prove integrity.

    Materialization itself must not depend on git lfs checkout: the verified
    object cache is already the content authority. In a real Git worktree we
    additionally prove that the configured LFS clean filter maps the bytes back
    to the tracked pointer, so the checkout stays clean. In isolated unit-test
    workspaces there may be no Git repository at all, and byte identity is the
    complete contract.
    """
    target = workspace / relative
    cached = lfs_object_path(workspace, oid)
    if not cached.exists():
        raise RuntimeError(f"LFS object cache missing for {relative}: {cached}")

    cached_hash = sha256_file(cached)
    if cached_hash != oid:
        raise RuntimeError(
            f"LFS object cache hash mismatch for {relative}: "
            f"actual={cached_hash} expected={oid}"
        )

    target.parent.mkdir(parents=True, exist_ok=True)
    temp_target = target.with_name(target.name + ".pinkcab-lfs-tmp")
    shutil.copy2(cached, temp_target)
    if sha256_file(temp_target) != oid:
        temp_target.unlink(missing_ok=True)
        raise RuntimeError(f"Temporary materialization hash mismatch for {relative}")
    os.replace(temp_target, target)

    actual = sha256_file(target)
    if actual != oid:
        raise RuntimeError(
            f"Materialized fixture hash mismatch for {relative}: "
            f"actual={actual} expected={oid}"
        )

    if not is_git_worktree(workspace):
        return

    diff = subprocess.run(
        ["git", "diff", "--quiet", "--", relative.as_posix()],
        cwd=workspace,
        check=False,
    )
    if diff.returncode not in (0, 1):
        raise RuntimeError(
            f"git diff failed while verifying materialized fixture: {relative}"
        )
    if diff.returncode != 0:
        raise RuntimeError(
            f"Materialized fixture differs from Git index after LFS clean filter: {relative}"
        )

    # Content equivalence is already proven above through Git's configured LFS
    # clean filter. Refresh only the index stat cache so status does not report
    # a false modification solely because we atomically replaced the pointer
    # with its canonical smudged bytes.
    refresh = subprocess.run(
        ["git", "update-index", "--refresh", "--", relative.as_posix()],
        cwd=workspace,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if refresh.returncode != 0:
        print(
            f"PINKCAB_LOCAL_LFS_INDEX_REFRESH_NONZERO={relative} "
            f"code={refresh.returncode} "
            f"stdout={refresh.stdout.strip()} stderr={refresh.stderr.strip()}"
        )

    # The semantic diff above is the content-equivalence authority because it
    # invokes the configured LFS clean filter. Index refresh is only a stat-cache
    # optimization and may report needs-update for a newly smudged LFS path.
    # Final porcelain status below remains a hard cleanliness gate.
    status = subprocess.run(
        ["git", "status", "--porcelain", "--", relative.as_posix()],
        cwd=workspace,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        check=False,
    )
    if status.returncode != 0 or status.stdout.strip():
        raise RuntimeError(
            f"Materialized fixture dirties Git worktree: {relative} "
            f"status={status.stdout.strip()}"
        )


def materialize_required(
    workspace: Path,
    relative: Path,
    search_roots: list[Path],
    cache_roots: list[Path],
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
        materialize_cached_lfs_object(workspace, relative, oid)
        print(f"PINKCAB_LOCAL_LFS_FROM_OBJECT_CACHE={relative} oid={oid}")
        return

    matches: list[Path] = []

    # Search exact historical OIDs across trusted local clone/cache roots
    # before falling back to current worktree files.
    for external_cached in iter_nested_lfs_object_candidates(
        cache_roots, oid, workspace
    ):
        try:
            actual = sha256_file(external_cached)
        except OSError:
            continue
        print(
            f"PINKCAB_LOCAL_LFS_NESTED_OBJECT_CANDIDATE={external_cached} "
            f"sha256={actual} expected={oid}"
        )
        if actual == oid:
            matches.append(external_cached)

    # Historical LFS object caches are authoritative candidates too. A mirror
    # worktree may have moved on to a newer revision while its local LFS store
    # still retains the exact OID required by this candidate SHA.
    for root in search_roots:
        if not root.exists():
            continue
        external_cached = external_lfs_object_path(root, oid)
        if external_cached is None or not external_cached.exists():
            continue
        try:
            actual = sha256_file(external_cached)
        except OSError:
            continue
        print(
            f"PINKCAB_LOCAL_LFS_OBJECT_CANDIDATE={external_cached} "
            f"sha256={actual} expected={oid}"
        )
        if actual == oid:
            matches.append(external_cached)

    # Fast path: canonical mirrors preserve repository-relative paths.
    for root in search_roots:
        candidate = root / relative
        if not candidate.exists():
            continue
        if candidate.resolve() == target.resolve():
            continue
        if parse_lfs_pointer(candidate) is not None:
            continue
        try:
            actual = sha256_file(candidate)
        except OSError:
            continue
        print(
            f"PINKCAB_LOCAL_LFS_DIRECT_CANDIDATE={candidate} "
            f"sha256={actual} expected={oid}"
        )
        if actual == oid:
            matches.append(candidate)

    # Fallback is only needed for legacy mirrors whose root does not preserve
    # the current repository-relative layout.
    if not matches:
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

    materialize_cached_lfs_object(workspace, relative, oid)

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
    parser.add_argument("--all-tracked", action="store_true")
    parser.add_argument("--search-root", action="append", default=[])
    parser.add_argument("--cache-root", action="append", default=[])
    args = parser.parse_args()

    workspace = args.workspace.resolve()
    requested = [Path(value) for value in args.required]
    roots = [Path(value) for value in args.search_root]
    cache_roots = [Path(value) for value in args.cache_root]

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

    required = (
        [Path(value) for value in sorted(tracked)]
        if args.all_tracked
        else requested
    )
    for relative in required:
        materialize_required(workspace, relative, roots, cache_roots)

    unresolved: list[str] = []
    for relative in sorted(tracked):
        path = workspace / relative
        if not path.exists() or parse_lfs_pointer(path) is not None:
            unresolved.append(relative)

    for relative in required:
        path = workspace / relative
        if not path.exists() or parse_lfs_pointer(path) is not None:
            raise RuntimeError(f"Required fixture remains unresolved: {relative}")

    if args.all_tracked and unresolved:
        raise RuntimeError(
            f"Full LFS materialization incomplete: {len(unresolved)} unresolved"
        )

    print(
        "PINKCAB_LOCAL_LFS=PASS "
        f"required={len(required)} tracked={len(tracked)} "
        f"unresolved_unrelated={len(unresolved)} all_tracked={int(args.all_tracked)}"
    )
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as exc:
        print(f"PINKCAB_LOCAL_LFS=FAIL {exc}", file=sys.stderr)
        raise
