"""Exact-source scope, warm-workspace and same-run physics evidence contracts."""
from __future__ import annotations
import argparse
import hashlib
import itertools
import json
import os
from pathlib import Path
import re
import subprocess


def git(root: Path, *args: str) -> str:
    return subprocess.run(['git', *args], cwd=root, check=True,
                          capture_output=True, text=True).stdout.strip()


def runtime_required(paths: list[str]) -> bool:
    # Only known prose/administrative changes can bypass Unreal. Unknown files,
    # workflow scripts and the executable writer inventory always require it.
    administrative = {'README.md', 'CONTRIBUTING.md', 'LICENSE', 'LICENSE.md'}
    for path in paths:
        if path == 'docs/vehicle_physics/STATE_WRITER_INVENTORY.csv':
            return True
        if path in administrative or path.startswith('docs/'):
            continue
        return True
    return False


def scope(root: Path, event: str, base: str, expected: str) -> bool:
    if not re.fullmatch(r'[0-9a-f]{40}', expected) or git(root, 'rev-parse', 'HEAD') != expected:
        raise ValueError('PINKCAB_SCOPE_HEAD_MISMATCH')
    if event == 'workflow_dispatch' or (event == 'push' and base == '0' * 40):
        return True
    if event not in {'pull_request', 'push'}:
        raise ValueError('PINKCAB_SCOPE_UNSUPPORTED_EVENT')
    if not re.fullmatch(r'[0-9a-f]{40}', base):
        raise ValueError('PINKCAB_SCOPE_BASE_INVALID')
    git(root, 'cat-file', '-e', base + '^{commit}')
    # Compare the full PR change set, not merely its last commit. Pushes use the
    # complete before..after range. Missing history fails instead of skipping.
    compare = git(root, 'merge-base', base, expected) if event == 'pull_request' else base
    paths = git(root, 'diff', '--name-only', '--no-renames', '-z', compare, expected).split('\0')
    paths = [p for p in paths if p]
    print(json.dumps({'candidate_sha': expected, 'scope_base': compare, 'changed': paths}))
    return runtime_required(paths)


def workspace(root: Path, expected: str) -> None:
    if not re.fullmatch(r'[0-9a-f]{40}', expected) or git(root, 'rev-parse', 'HEAD') != expected:
        raise ValueError('PINKCAB_WORKSPACE_HEAD_MISMATCH')
    if git(root, 'status', '--porcelain', '--untracked-files=all'):
        raise ValueError('PINKCAB_WORKSPACE_DIRTY')
    # Git ignores caches, but ignored files inside authored input roots can
    # still alter compilation/cooking. Reject these too, except pinned vendor
    # content and strictly designated per-plugin Unreal output directories.
    ignored = git(root, 'ls-files', '--others', '--ignored', '--exclude-standard', '-z',
                  '--', 'Source', 'Config', 'Content', 'Build', 'Plugins').split('\0')
    vendor = root / 'Plugins/MetaRoad'
    if vendor.exists():
        try:
            from .prepare_metaroad import verify
        except ImportError:  # Direct command-line execution.
            from prepare_metaroad import verify
        verify(vendor, json.loads((root / 'scripts/ci/metaroad.lock.json').read_text()))
    generated = {'Binaries', 'Intermediate', 'Saved', 'DerivedDataCache'}
    unexpected = []
    for path in filter(None, ignored):
        parts = Path(path).parts
        if path.startswith('Plugins/MetaRoad/') and vendor.exists():
            continue  # Entire authored package was verified against the lock.
        if len(parts) >= 4 and parts[0] == 'Plugins' and parts[2] in generated:
            continue
        unexpected.append(path)
    if unexpected:
        raise ValueError('PINKCAB_WORKSPACE_IGNORED_AUTHORED_FILES=' + ','.join(unexpected[:10]))


def expected_tests(root: Path) -> set[str]:
    names = set()
    for path in (root / 'Source/PinkCabTests/Private/Vehicle').glob('*.cpp'):
        names.update(re.findall(r'IMPLEMENT_\w+_AUTOMATION_TEST\s*\(\s*\w+\s*,\s*"(PinkCab\.Vehicle\.Physics[^"\s]*)"', path.read_text(encoding='utf-8')))
    if not names or 'PinkCab.Vehicle.Physics.P02.D3.CausalMatrix' not in names:
        raise ValueError('PINKCAB_PHYSICS_EXPECTED_TESTS_MISSING')
    return names


def log_evidence(root: Path, log: Path) -> dict:
    text = log.read_text(encoding='utf-8', errors='replace')
    if re.search(r'Test Completed\. Result=\{Fail\}|LogAutomationController: Error:', text):
        raise ValueError('PINKCAB_PHYSICS_FAILURE_IN_LOG')
    expected = expected_tests(root)
    passed = set(re.findall(r'Test Completed\. Result=\{Success\}.*?Path=\{(PinkCab\.Vehicle\.Physics[^}]+)\}', text))
    if expected != passed:
        raise ValueError(f'PINKCAB_PHYSICS_COVERAGE_MISMATCH missing={sorted(expected-passed)} extra={sorted(passed-expected)}')
    queues = re.findall(r'Automation Test Queue Empty (\d+) tests performed', text)
    if not queues or int(queues[-1]) != len(expected):
        raise ValueError('PINKCAB_PHYSICS_QUEUE_COUNT_MISMATCH')
    samples = re.findall(r'P02_D3_MATRIX coupling=([\d.]+) gear=(-?\d+) throttle=([\d.]+) repeat=(\d+)', text)
    expected_samples = set(itertools.product(
        ('0.000', '0.250', '0.500', '0.750', '0.999', '1.000'),
        ('1', '-1'), ('0.00', '0.25', '0.50', '1.00'), ('1', '2', '3', '4', '5')))
    if len(samples) != 240 or set(samples) != expected_samples:
        raise ValueError('PINKCAB_PHYSICS_D3_MATRIX_INCOMPLETE')
    return {'passed_tests': sorted(passed), 'd3_unique_samples': len(set(samples)),
            'log_sha256': hashlib.sha256(log.read_bytes()).hexdigest()}


def attest(root: Path, log: Path, manifest: Path, sha: str, run: str, attempt: str) -> None:
    if git(root, 'rev-parse', 'HEAD') != sha:
        raise ValueError('PINKCAB_ATTESTATION_HEAD_MISMATCH')
    evidence = log_evidence(root, log)
    evidence.update(schema=1, candidate_sha=sha, run_id=run, run_attempt=attempt,
                    result='success', suite='PinkCab.Vehicle.Physics')
    manifest.parent.mkdir(parents=True, exist_ok=True)
    manifest.write_text(json.dumps(evidence, indent=2) + '\n', encoding='utf-8')


def verify_attestation(root: Path, log: Path, manifest: Path, sha: str, run: str, attempt: str) -> None:
    receipt = json.loads(manifest.read_text(encoding='utf-8'))
    for key, value in dict(schema=1, candidate_sha=sha, run_id=run, run_attempt=attempt,
                           result='success', suite='PinkCab.Vehicle.Physics').items():
        if receipt.get(key) != value:
            raise ValueError('PINKCAB_ATTESTATION_MISMATCH=' + key)
    evidence = log_evidence(root, log)
    if any(receipt.get(k) != v for k, v in evidence.items()):
        raise ValueError('PINKCAB_ATTESTATION_EVIDENCE_MISMATCH')


def gate(required: str, verification: str, tdd: str, p02: str) -> str:
    if verification != 'success' or required not in {'true', 'false'}:
        raise ValueError('PINKCAB_ACCEPTANCE_STATIC_OR_SCOPE_FAILED')
    if required == 'true':
        if tdd != 'success' or p02 != 'success':
            raise ValueError('PINKCAB_ACCEPTANCE_RUNTIME_FAILED_OR_MISSING')
        return 'PASS: exact-candidate physics and P02 repeat evidence; human acceptance remains separate'
    if tdd != 'skipped' or p02 != 'skipped':
        raise ValueError('PINKCAB_ACCEPTANCE_DOCS_SCOPE_INCONSISTENT')
    return 'N/A: verified administrative-only change; no new gameplay acceptance'


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('command', choices=['scope', 'workspace', 'attest', 'verify-attestation', 'gate'])
    parser.add_argument('--root', type=Path, default=Path.cwd())
    parser.add_argument('--sha', default=os.environ.get('PINKCAB_CANDIDATE_SHA', ''))
    parser.add_argument('--log', type=Path)
    parser.add_argument('--manifest', type=Path)
    args = parser.parse_args()
    try:
        if args.command == 'scope':
            event = os.environ['GITHUB_EVENT_NAME']
            base = os.environ.get('PINKCAB_PR_BASE_SHA' if event == 'pull_request' else 'PINKCAB_PUSH_BEFORE', '')
            required = scope(args.root, event, base, args.sha)
            with open(os.environ['GITHUB_OUTPUT'], 'a', encoding='utf-8') as output:
                output.write(f'run_physics={str(required).lower()}\n')
        elif args.command == 'workspace':
            workspace(args.root, args.sha)
        elif args.command in {'attest', 'verify-attestation'}:
            fn = attest if args.command == 'attest' else verify_attestation
            fn(args.root, args.log, args.manifest, args.sha, os.environ['GITHUB_RUN_ID'], os.environ['GITHUB_RUN_ATTEMPT'])
        else:
            print(gate(*(os.environ.get(key, '') for key in ('PINKCAB_RUNTIME_REQUIRED', 'PINKCAB_STATIC_RESULT', 'PINKCAB_TDD_RESULT', 'PINKCAB_P02_RESULT'))))
    except (ValueError, OSError, subprocess.CalledProcessError) as error:
        print(f'PINKCAB_ACCEPTANCE_CONTRACT=FAIL {error}')
        return 1
    print('PINKCAB_ACCEPTANCE_CONTRACT=PASS')
    return 0


if __name__ == '__main__':
    raise SystemExit(main())
