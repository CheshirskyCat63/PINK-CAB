"""Preserve and retire only generated changes from the verified CD-869 delivery."""
from __future__ import annotations
import hashlib, json, os, shutil, subprocess, sys
from pathlib import Path, PurePosixPath


def git(root: Path, *args: str, env: dict[str, str] | None = None) -> bytes:
    p = subprocess.run(['git', '-C', str(root), *args], capture_output=True, env=env)
    if p.returncode:
        raise RuntimeError(f'Git {args[0]} failed: {p.stderr.decode("utf-8", "replace")[:500]}')
    return p.stdout


def digest(path: Path) -> str:
    h = hashlib.sha256()
    with path.open('rb') as stream:
        for block in iter(lambda: stream.read(1024 * 1024), b''):
            h.update(block)
    return h.hexdigest()


def owned_generated(name: str) -> bool:
    return (name.startswith('Content/World/L1/Road/')
            or name == 'Content/Dev/Authoring/L_PC_L1_MetaRoadAuthoring.umap')


def safe_file(root: Path, name: str) -> Path:
    relative = PurePosixPath(name)
    if relative.is_absolute() or '..' in relative.parts or not name or '\\' in name:
        raise RuntimeError('Invalid repository-relative path')
    path = root.joinpath(*relative.parts)
    if not path.resolve().is_relative_to(root.resolve()):
        raise RuntimeError('Path escapes the owned runner workspace')
    cursor = path
    while cursor != root:
        if cursor.exists() and (cursor.is_symlink() or getattr(cursor.stat(), 'st_file_attributes', 0) & 0x400):
            raise RuntimeError('Reparse path is not allowed')
        cursor = cursor.parent
    return path


def inspect(root: Path, sha: str) -> tuple[bytes, list[dict]]:
    if git(root, 'rev-parse', 'HEAD').decode().strip() != sha:
        raise RuntimeError('Runner HEAD changed')
    if git(root, 'diff', '--cached', '--name-only'):
        raise RuntimeError('Staged changes are not owned by this seal operation')
    status = git(root, 'status', '--porcelain=v1', '-z', '--untracked-files=all')
    rows = []
    for entry in status.split(b'\0'):
        if not entry:
            continue
        state, name = entry[:2].decode('ascii'), entry[3:].decode('utf-8')
        if state not in (' M', ' D', '??'):
            raise RuntimeError(f'Unexpected change type {state!r}; preserve without reset')
        source = safe_file(root, name)
        if state == ' D':
            original = git(root, 'show', f'{sha}:{name}')
            if not original.startswith(b'version https://git-lfs.github.com/spec/v1\n') or len(original) > 512:
                raise RuntimeError('Deleted non-LFS source is not eligible for regeneration')
        elif not owned_generated(name):
            raise RuntimeError(f'Unexpected authored change {name}; preserve without reset')
        rows.append({'state': state, 'path': name, 'bytes': source.stat().st_size if source.exists() else None,
                     'sha256': digest(source) if source.exists() else None})
    return status, rows


def seal(root: Path, output: Path, sha: str) -> dict:
    root = root.resolve()
    if output.exists() or output.resolve().is_relative_to(root):
        raise RuntimeError('Evidence location must be new and outside the runner checkout')
    status, rows = inspect(root, sha)
    output.mkdir(parents=True)
    (output / 'status-before.bin').write_bytes(status)
    (output / 'generated-diff.patch').write_bytes(git(root, 'diff', '--binary', sha))
    for row in rows:
        source = safe_file(root, row['path'])
        if not source.exists():
            continue
        target = output / 'generated-inputs' / row['path']
        target.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source, target)
        if digest(target) != row['sha256'] or digest(source) != row['sha256']:
            raise RuntimeError('Generated input changed while being preserved')
    manifest = {'candidate_sha': sha, 'scope': 'owned CD-869 generated changes only',
                'rows': rows, 'result': 'PRESERVED_NOT_YET_RESTORED'}
    (output / 'generated-inputs.json').write_text(json.dumps(manifest, ensure_ascii=False, indent=2), encoding='utf-8')
    status_now, rows_now = inspect(root, sha)
    if status_now != status or rows_now != rows:
        raise RuntimeError('Concurrent workspace change; retained evidence, no restore attempted')
    tracked = [row['path'] for row in rows if row['state'] != '??']
    if tracked:
        paths = output / 'tracked-paths.bin'
        paths.write_bytes(b'\0'.join(p.encode('utf-8') for p in tracked) + b'\0')
        git(root, 'restore', f'--source={sha}', '--worktree', f'--pathspec-from-file={paths}',
            '--pathspec-file-nul', env={**os.environ, 'GIT_LFS_SKIP_SMUDGE': '1', 'GIT_TERMINAL_PROMPT': '0'})
    for row in rows:
        if row['state'] == '??':
            source = safe_file(root, row['path'])
            if digest(source) != row['sha256']:
                raise RuntimeError('Untracked generated input changed; not moved')
            target = output / 'retained-untracked-originals' / row['path']
            target.parent.mkdir(parents=True, exist_ok=True)
            os.replace(source, target)
            if digest(target) != row['sha256']:
                raise RuntimeError('Untracked generated-input move failed verification')
    after = git(root, 'status', '--porcelain=v1', '-z', '--untracked-files=all')
    (output / 'status-after.bin').write_bytes(after)
    if after:
        raise RuntimeError('Workspace remains dirty; evidence preserved')
    manifest['result'] = 'GENERATED_INPUTS_RETAINED_RUNNER_CLEAN'
    manifest['restored_tracked_paths'] = len(tracked)
    manifest['retained_untracked_paths'] = sum(r['state'] == '??' for r in rows)
    manifest['original_game_packages_touched'] = False
    manifest['canonical_user_checkout_touched'] = False
    (output / 'seal.json').write_text(json.dumps(manifest, ensure_ascii=False, indent=2), encoding='utf-8')
    return manifest


if __name__ == '__main__':
    root, output, sha = Path(sys.argv[1]), Path(sys.argv[2]), sys.argv[3]
    result = seal(root, output, sha)
    print(json.dumps({k: v for k, v in result.items() if k != 'rows'}, ensure_ascii=False))
