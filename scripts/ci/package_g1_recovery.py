#!/usr/bin/env python3
"""Package the preserved G1 planning archive from one immutable Git commit."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import zipfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--source-ref', default='HEAD')
    parser.add_argument('--output', type=Path, default=Path('Artifacts/PINKCAB_G1_RECOVERY.zip'))
    args = parser.parse_args()
    root = Path(__file__).resolve().parents[2]
    def git(*arguments):
        return subprocess.check_output(['git', '-C', str(root), *arguments])
    sha = git('rev-parse', '--verify', f'{args.source_ref}^{{commit}}').decode().strip()
    prefix = 'docs/archive/g1-recovery-2026-09-20/'
    paths = git('ls-tree', '-r', '--name-only', sha, prefix).decode().splitlines()
    files = {path[len(prefix):]: git('show', f'{sha}:{path}') for path in paths}
    tasks = json.loads(files['tasks.json'])['tasks']
    assets = json.loads(files['assets.json'])['assets']
    sources = json.loads(files['sources/index.json'])['sources']
    requirements = json.loads(files['PINKCAB_G1_REQUIREMENTS_2026-09-20.json'])['requirements']
    for name, rows, expected in [('tasks', tasks, 83), ('assets', assets, 159), ('sources', sources, 32), ('requirements', requirements, 42)]:
        if len(rows) != expected:
            raise ValueError(f'{name}: expected {expected}, found {len(rows)}')
    by_id = {task['id']: task for task in tasks}
    if len(by_id) != 83 or len({asset['id'] for asset in assets}) != 159:
        raise ValueError('Duplicate recovery IDs')
    visiting, visited = set(), set()
    def visit(task_id):
        if task_id in visiting or task_id not in by_id:
            raise ValueError(f'Invalid dependency/cycle: {task_id}')
        if task_id in visited:
            return
        visiting.add(task_id)
        for dependency in by_id[task_id]['depends']:
            visit(dependency)
        visiting.remove(task_id)
        visited.add(task_id)
    for task_id in by_id:
        visit(task_id)
    manifest = {
        'schema': 'pinkcab.g1.recovery.package.v1', 'source_commit': sha,
        'package_version': '2026-10-03', 'authority': 'HISTORICAL_PLANNING_ARCHIVE_NOT_LIVE_BACKLOG',
        'counts': {'tasks': 83, 'assets': 159, 'requirements': 42, 'sources': 32},
        'fidelity': {'original_zip': 'MISSING_SOURCE_NEVER_MATERIALIZED',
                     'task_bodies': '5 exact/partial; 78 structural-only; see per-record body_recovery_state',
                     'file_bytes': 'EXACT_COPY_OF_PRESERVED_GIT_ARCHIVE',
                     'private_source_binaries': 'INDEXED_ONLY_NOT_INCLUDED'},
        'files': [{'path': name, 'sha256': hashlib.sha256(data).hexdigest(),
                   'state': 'PRESERVED_RECOVERY_COPY_SEE_SOURCE_FIDELITY'} for name, data in sorted(files.items())]
    }
    files['RECOVERY_MANIFEST.json'] = (json.dumps(manifest, indent=2) + '\n').encode()
    files['SHA256SUMS'] = ''.join(f'{hashlib.sha256(data).hexdigest()}  {name}\n' for name, data in sorted(files.items())).encode()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    with zipfile.ZipFile(args.output, 'w', compression=zipfile.ZIP_DEFLATED) as archive:
        for name, data in sorted(files.items()):
            entry = zipfile.ZipInfo(name, date_time=(2026, 10, 3, 0, 0, 0))
            entry.compress_type = zipfile.ZIP_DEFLATED
            archive.writestr(entry, data)
    with zipfile.ZipFile(args.output) as archive:
        if archive.testzip() is not None:
            raise ValueError('ZIP integrity failure')
        for line in archive.read('SHA256SUMS').decode().splitlines():
            digest, name = line.split('  ', 1)
            if hashlib.sha256(archive.read(name)).hexdigest() != digest:
                raise ValueError(f'Packaged hash mismatch: {name}')
    print(json.dumps({'result': 'PASS', 'source_commit': sha, 'files': len(files),
                      'sha256': hashlib.sha256(args.output.read_bytes()).hexdigest(), 'path': str(args.output)}))


if __name__ == '__main__':
    main()
