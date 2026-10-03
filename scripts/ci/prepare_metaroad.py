#!/usr/bin/env python3
"""Provision PINKCAB's pinned project plugin without changing the shared engine."""
from __future__ import annotations
import argparse
import hashlib
import json
import os
from pathlib import Path
import shutil
import tempfile


def verify(root: Path, lock: dict) -> None:
    expected = set()
    for entry in lock['files']:
        relative = Path(entry['path'])
        if relative.is_absolute() or '..' in relative.parts:
            raise ValueError('Unsafe vendor lock path')
        expected.add(relative.as_posix())
        path = root / relative
        if not path.is_file():
            raise ValueError(f'MetaRoad missing pinned file: {relative}')
        if hashlib.sha256(path.read_bytes()).hexdigest() != entry['sha256']:
            raise ValueError(f'MetaRoad hash mismatch: {relative}')
    # Ignore only top-level Unreal build/cache output and the source package's
    # provenance receipts. All authored roots (including unknown ones) are pinned.
    generated = {'binaries', 'intermediate', 'saved', 'deriveddatacache'}
    receipts = {'vendor.manifest', 'VERIFIED_PACKAGE.json'}
    extras = []
    for directory, children, names in os.walk(root):
        if Path(directory) == root:
            children[:] = [name for name in children if name.casefold() not in generated]
        for name in names:
            relative = (Path(directory) / name).relative_to(root).as_posix()
            if relative not in expected and relative not in receipts:
                extras.append(relative)
    if extras:
        raise ValueError(f'MetaRoad unexpected authored files (mixed installation): {sorted(extras)[:8]}')


def provision(repo: Path, source: Path, lock: dict) -> None:
    target = repo / 'Plugins' / 'MetaRoad'
    if target.exists():
        verify(target, lock)
        return
    verify(source, lock)
    staging_root = repo / 'Saved' / 'VendorPreparation'
    staging_root.mkdir(parents=True, exist_ok=True)
    staging = Path(tempfile.mkdtemp(prefix='metaroad-', dir=staging_root))
    for entry in lock['files']:
        destination = staging / entry['path']
        destination.parent.mkdir(parents=True, exist_ok=True)
        shutil.copy2(source / entry['path'], destination)
    verify(staging, lock)
    target.parent.mkdir(parents=True, exist_ok=True)
    staging.rename(target)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, default=Path(__file__).resolve().parents[2])
    parser.add_argument('--source', type=Path, default=Path(os.environ.get(
        'PINKCAB_METAROAD_PACKAGE',
        r'E:\CHESHIRE_DIVISION\Shared\Vendor\MetaRoad\3.2.0-ox4rZYlcREy004nfGp7EHg')))
    args = parser.parse_args()
    lock = json.loads((args.root / 'scripts/ci/metaroad.lock.json').read_text(encoding='utf-8'))
    try:
        provision(args.root, args.source, lock)
    except (OSError, ValueError) as error:
        print(f'PINKCAB_METAROAD_PIN=FAIL {error}')
        return 1
    print(f"PINKCAB_METAROAD_PIN=PASS version={lock['version']} authored_files={len(lock['files'])}")
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
