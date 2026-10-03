"""Negative tests for cached workspace and exact-run acceptance boundaries."""
import json
import itertools
import os
from pathlib import Path
import subprocess
import tempfile
import unittest

from scripts.ci import acceptance_contract as contract

ROOT = Path(__file__).resolve().parents[2]


class AcceptanceTests(unittest.TestCase):
    def test_scope_unknown_and_every_runtime_root_require_checks(self):
        for path in ['Source/new.cpp', 'Content/new.uasset', 'Config/default.ini',
                     'PinkCab.uproject', '.github/workflows/new.yml', 'new.unknown',
                     'docs/vehicle_physics/STATE_WRITER_INVENTORY.csv']:
            with self.subTest(path=path):
                self.assertTrue(contract.runtime_required(['README.md', path]))
        self.assertFalse(contract.runtime_required(['README.md', 'docs/CONTROL_PLANE.md']))

    def test_aggregate_rejects_failure_skip_cancellation_missing_scope(self):
        for outcome in ['failure', 'skipped', 'cancelled', '', 'neutral']:
            with self.subTest(outcome=outcome):
                with self.assertRaises(ValueError):
                    contract.gate('true', 'success', outcome, 'success')
                with self.assertRaises(ValueError):
                    contract.gate('true', 'success', 'success', outcome)
                with self.assertRaises(ValueError):
                    contract.gate('false', outcome, 'skipped', 'skipped')
        with self.assertRaises(ValueError):
            contract.gate('', 'success', 'skipped', 'skipped')
        self.assertTrue(contract.gate('false', 'success', 'skipped', 'skipped').startswith('N/A:'))
        self.assertTrue(contract.gate('true', 'success', 'success', 'success').startswith('PASS:'))

    def test_warm_workspace_preserves_cache_but_rejects_ignored_authored_inputs(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            def git(*args):
                return contract.git(root, *args)
            git('init', '-q')
            git('config', 'user.name', 'Acceptance Test')
            git('config', 'user.email', 'acceptance@example.invalid')
            (root / '.gitignore').write_text('Binaries/\nIntermediate/\nSaved/\n*.tmp\n')
            git('add', '.')
            git('commit', '-qm', 'baseline')
            sha = git('rev-parse', 'HEAD')
            (root / 'Binaries').mkdir()
            (root / 'Binaries/cache.dll').write_text('warm cache')
            contract.workspace(root, sha)
            (root / 'Source').mkdir()
            ignored = root / 'Source/stale.tmp'
            ignored.write_text('authored input hidden by gitignore')
            with self.assertRaisesRegex(ValueError, 'IGNORED_AUTHORED'):
                contract.workspace(root, sha)
            ignored.unlink()
            (root / 'Source/stale.cpp').write_text('untracked input')
            with self.assertRaisesRegex(ValueError, 'WORKSPACE_DIRTY'):
                contract.workspace(root, sha)

    def complete_log(self, root):
        tests = contract.expected_tests(ROOT)
        lines = [f'Test Completed. Result={{Success}} Name={{test}} Path={{{name}}}' for name in sorted(tests)]
        lines += [f'P02_D3_MATRIX coupling={coupling} gear={gear} throttle={throttle} repeat={repeat}'
                  for coupling, gear, throttle, repeat in itertools.product(
                      ('0.000', '0.250', '0.500', '0.750', '0.999', '1.000'),
                      ('1', '-1'), ('0.00', '0.25', '0.50', '1.00'), range(1, 6))]
        lines += [f'Automation Test Queue Empty {len(tests)} tests performed']
        log = root / 'suite.log'
        log.write_text('\n'.join(lines), encoding='utf-8')
        return log

    def test_full_coverage_receipt_rejects_missing_test_matrix_and_queue(self):
        with tempfile.TemporaryDirectory() as directory:
            log = self.complete_log(Path(directory))
            original = log.read_text()
            evidence = contract.log_evidence(ROOT, log)
            self.assertEqual(evidence['d3_unique_samples'], 240)
            self.assertGreaterEqual(len(evidence['passed_tests']), 47)
            for corrupt in [original.replace('Result={Success}', 'Result={Fail}', 1),
                            '\n'.join(original.splitlines()[1:]),
                            original.replace('repeat=5', 'repeat=4', 1),
                            original.replace('coupling=0.250', 'coupling=0.300'),
                            original.replace('Automation Test Queue Empty', 'No queue')]:
                log.write_text(corrupt)
                with self.assertRaises(ValueError):
                    contract.log_evidence(ROOT, log)

    def test_attestation_is_bound_to_sha_run_attempt_and_log_bytes(self):
        with tempfile.TemporaryDirectory() as directory:
            root = Path(directory)
            log = self.complete_log(root)
            evidence = contract.log_evidence(ROOT, log)
            evidence.update(schema=1, candidate_sha='a'*40, run_id='10', run_attempt='1',
                            result='success', suite='PinkCab.Vehicle.Physics')
            manifest = root / 'manifest.json'
            manifest.write_text(json.dumps(evidence))
            contract.verify_attestation(ROOT, log, manifest, 'a'*40, '10', '1')
            for sha, run, attempt in [('b'*40, '10', '1'), ('a'*40, '11', '1'), ('a'*40, '10', '2')]:
                with self.assertRaises(ValueError):
                    contract.verify_attestation(ROOT, log, manifest, sha, run, attempt)
            log.write_text(log.read_text() + '\nunrelated modification')
            with self.assertRaises(ValueError):
                contract.verify_attestation(ROOT, log, manifest, 'a'*40, '10', '1')

    def test_coordinator_single_suite_and_independent_five_slope_repeats(self):
        workflows = ROOT / '.github/workflows'
        tdd = (workflows / 'pinkcab-vehicle-physics-tdd.yml').read_text()
        p02 = (workflows / 'cd648-p02-phy009.yml').read_text()
        main = (workflows / 'pinkcab-repository-verification.yml').read_text()
        self.assertEqual(tdd.count("-TestName 'PinkCab.Vehicle.Physics'"), 1)
        self.assertNotIn("-TestName 'PinkCab.Vehicle.Physics.P02.D3.CausalMatrix'", tdd)
        self.assertIn('foreach($repeat in 1..5)', p02)
        self.assertIn('if: ${{ !inputs.reuse_tdd_evidence }}', p02)
        self.assertIn('needs: [verify, physics-tdd]', main)
        self.assertIn('name: Gameplay acceptance gate', main)
        self.assertIn('if: ${{ always() }}', main)
        self.assertIn('verify-attestation', p02)
        for text in (tdd, p02):
            self.assertIn('clean: false', text)
            self.assertNotIn('  pull_request:', text)
            self.assertIn('  workflow_call:', text)
            self.assertIn('  workflow_dispatch:', text)


if __name__ == '__main__':
    unittest.main()
