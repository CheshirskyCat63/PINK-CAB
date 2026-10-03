import pathlib
import os
import shutil
import subprocess
import tempfile
import re
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
WORKFLOW = ROOT / ".github" / "workflows" / "cd648-p02-phy009.yml"


class P02WorkflowScopeTests(unittest.TestCase):
    def setUp(self):
        self.text = WORKFLOW.read_text(encoding="utf-8")

    def test_docs_only_changes_do_not_enter_runtime_physics_scope(self):
        match = re.search(
            r"\$physics=@\(\$changed \| Where-Object \{(?P<body>.*?)\n\s*\}\)",
            self.text,
            flags=re.S,
        )
        self.assertIsNotNone(match, "change-scope classifier block not found")
        body = match.group("body")
        self.assertNotIn(
            "$_.StartsWith('docs/vehicle_physics/')",
            body,
            "docs-only admin changes must not trigger the P02 runtime suite",
        )

    def test_scope_regression_file_is_a_pull_request_trigger(self):
        self.assertIn(
            "- 'scripts/tests/test_p02_workflow_scope.py'",
            self.text,
            "editing the scope regression test must trigger this workflow",
        )

    def test_runtime_scope_guard_allows_canonical_p02_admin_documents(self):
        required = (
            "docs/vehicle_physics/P02_DRIVELINE_ARCHITECTURE_2026-09-28.md",
            "docs/vehicle_physics/PINK_CAB_VEHICLE_PHYSICS_CALIBRATION_PROGRAM_2026-09-26.md",
            "docs/vehicle_physics/TASKS.csv",
            "docs/vehicle_physics/TESTS.csv",
            "docs/vehicle_physics/GATE1_ROAD_HANDLING_INTEGRATION_2026-09-28.md",
        )
        allowed_match = re.search(
            r"\$allowed=@\((?P<body>.*?)\n\s*\)",
            self.text,
            flags=re.S,
        )
        self.assertIsNotNone(allowed_match, "scope-guard allowlist block not found")
        allowed = allowed_match.group("body")
        for path in required:
            with self.subTest(path=path):
                self.assertIn(path, allowed)



WORKFLOWS = ROOT / ".github" / "workflows"
PHYSICS_WORKFLOWS = ("cd648-p02-phy009.yml", "pinkcab-vehicle-physics-tdd.yml")
POWERSHELL = shutil.which("powershell") or shutil.which("pwsh")


def marked_script(path, name):
    text = path.read_text(encoding="utf-8")
    start = f"          # {name}_BEGIN\n"
    end = f"          # {name}_END"
    body = text.split(start, 1)[1].split(end, 1)[0]
    return "$ErrorActionPreference='Stop'\n" + "\n".join(line[10:] for line in body.splitlines())


class WorkflowTrustTests(unittest.TestCase):
    def test_pull_request_jobs_cannot_enter_self_hosted_runner_from_forks(self):
        for name in (*PHYSICS_WORKFLOWS, "cd869-deliver.yml"):
            text = (WORKFLOWS / name).read_text(encoding="utf-8")
            jobs = text.split("\njobs:\n", 1)[1]
            for match in re.finditer(r"(?ms)^  ([\w-]+):\n(.*?)(?=^  [\w-]+:|\Z)", jobs):
                job, body = match.groups()
                if "runs-on: [self-hosted" in body:
                    with self.subTest(workflow=name, job=job):
                        guard = re.search(r"(?m)^    if: (.+)$", body)
                        self.assertIsNotNone(guard, "trust check must be at job scheduling level")
                        self.assertIn("github.event_name != 'pull_request' ||", guard.group(1))
                        self.assertIn("github.event.pull_request.head.repo.full_name == github.repository", guard.group(1))
                        self.assertIn("github.event.pull_request.author_association", guard.group(1))
                        self.assertIn('["OWNER","MEMBER","COLLABORATOR"]', guard.group(1))
                        self.assertNotIn('"CONTRIBUTOR"', guard.group(1))

    def test_human_delivery_requires_explicit_dispatch(self):
        delivery = (WORKFLOWS / "cd869-deliver.yml").read_text(encoding="utf-8")
        triggers = delivery.split("\npermissions:", 1)[0]
        self.assertNotIn("  pull_request:", triggers)
        self.assertIn("  workflow_dispatch:", triggers)
        p02 = (WORKFLOWS / "cd648-p02-phy009.yml").read_text(encoding="utf-8")
        self.assertIn("github.event_name == 'workflow_dispatch' && inputs.deliver_human", p02)
        self.assertIn("default: false", p02)

    def test_general_verification_covers_all_pull_requests_on_hosted_runner(self):
        text = (WORKFLOWS / "pinkcab-repository-verification.yml").read_text(encoding="utf-8")
        trigger = text.split("  pull_request:\n", 1)[1].split("  push:", 1)[0]
        self.assertNotIn("paths:", trigger)
        self.assertIn("runs-on: windows-latest", text)
        self.assertNotIn("runs-on: [self-hosted", text)
        self.assertIn("persist-credentials: false", text)


@unittest.skipUnless(POWERSHELL, "PowerShell is required for workflow execution regressions")
class WorkflowExecutionTests(unittest.TestCase):
    def setUp(self):
        self.temp = tempfile.TemporaryDirectory()
        self.addCleanup(self.temp.cleanup)
        self.repo = pathlib.Path(self.temp.name) / "repo"
        self.repo.mkdir()
        self.git("init", "-q")
        self.git("config", "user.name", "Workflow Test")
        self.git("config", "user.email", "workflow-test@example.invalid")
        self.write("docs/readme.md", "baseline")
        self.write("Config/DefaultEngine.ini", "baseline")
        self.base = self.commit("baseline")

    def git(self, *args):
        return subprocess.run(["git", *args], cwd=self.repo, text=True, check=True,
                              stdout=subprocess.PIPE, stderr=subprocess.PIPE).stdout.strip()

    def write(self, relative, content):
        path = self.repo / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")

    def commit(self, message):
        self.git("add", ".")
        self.git("commit", "-qm", message)
        return self.git("rev-parse", "HEAD")

    def execute(self, script, **env):
        path = pathlib.Path(self.temp.name) / "workflow.ps1"
        path.write_text(script, encoding="utf-8")
        return subprocess.run([POWERSHELL, "-NoProfile", "-NonInteractive", "-ExecutionPolicy", "Bypass", "-File", str(path)],
                              cwd=self.repo, env={**os.environ, **env}, text=True,
                              stdout=subprocess.PIPE, stderr=subprocess.STDOUT, timeout=30)

    def scope(self, workflow, event, before=None):
        output = pathlib.Path(self.temp.name) / "scope.txt"
        output.unlink(missing_ok=True)
        result = self.execute(marked_script(WORKFLOWS / workflow, "PINKCAB_CHANGE_SCOPE"),
                              GITHUB_EVENT_NAME=event, PINKCAB_PR_BASE_SHA=self.base,
                              PINKCAB_PUSH_BEFORE=before or self.base, GITHUB_OUTPUT=str(output))
        self.assertEqual(result.returncode, 0, result.stdout)
        return output.read_text(encoding="utf-8-sig").strip()

    def test_physics_commit_followed_by_docs_still_runs_for_entire_pr(self):
        self.write("Source/PinkCabVehicle/Private/Changed.cpp", "// physics")
        self.commit("physics change")
        self.write("docs/readme.md", "documentation follow-up")
        self.commit("docs change")
        for workflow in PHYSICS_WORKFLOWS:
            with self.subTest(workflow=workflow):
                self.assertEqual(self.scope(workflow, "pull_request"), "run_physics=true")

    def test_push_scope_includes_all_pushed_commits(self):
        self.write("Source/PinkCabVehicle/Private/Changed.cpp", "// physics")
        self.commit("physics change")
        self.write("docs/readme.md", "documentation follow-up")
        self.commit("docs change")
        for workflow in PHYSICS_WORKFLOWS:
            with self.subTest(workflow=workflow):
                self.assertEqual(self.scope(workflow, "push"), "run_physics=true")

    def test_docs_only_pr_skips_physics_but_manual_and_new_branch_force_it(self):
        self.write("docs/readme.md", "documentation only")
        self.commit("docs change")
        for workflow in PHYSICS_WORKFLOWS:
            with self.subTest(workflow=workflow):
                self.assertEqual(self.scope(workflow, "pull_request"), "run_physics=false")
                self.assertEqual(self.scope(workflow, "workflow_dispatch"), "run_physics=true")
                self.assertEqual(self.scope(workflow, "push", "0" * 40), "run_physics=true")

    def test_missing_pr_base_fails_instead_of_reporting_green(self):
        for workflow in PHYSICS_WORKFLOWS:
            with self.subTest(workflow=workflow):
                result = self.execute(marked_script(WORKFLOWS / workflow, "PINKCAB_CHANGE_SCOPE"),
                                      GITHUB_EVENT_NAME="pull_request", PINKCAB_PR_BASE_SHA="")
                self.assertNotEqual(result.returncode, 0, result.stdout)
                self.assertIn("PINKCAB_SCOPE_BASE_INVALID", result.stdout)

    def test_p02_preflight_accepts_reviewed_admin_files_but_rejects_unrelated_runtime(self):
        self.git("update-ref", "refs/remotes/origin/main", self.base)
        for path in (
            "README.md", "docs/AUTHORITY.yaml", "docs/PROJECT_SETUP.md", "docs/README.md",
            "scripts/ci/package_g1_recovery.py",
            "scripts/tests/test_ci_powershell_environment.py",
            "scripts/tests/test_package_g1_recovery.py",
        ):
            self.write(path, "administrative fixture")
        self.commit("reviewed admin change set")
        script = marked_script(WORKFLOWS / "cd648-p02-phy009.yml", "PINKCAB_P02_PREFLIGHT")
        accepted = self.execute(script)
        self.assertEqual(accepted.returncode, 0, accepted.stdout)
        self.assertIn("P02_PHY009_PREFLIGHT=PASS", accepted.stdout)
        self.write("Source/PinkCabVehicle/Private/UnrelatedRuntime.cpp", "// unrelated change")
        self.commit("unreviewed runtime scope")
        rejected = self.execute(script)
        self.assertNotEqual(rejected.returncode, 0, rejected.stdout)
        self.assertIn("P02_PHY009_SCOPE_GUARD_FAIL", rejected.stdout)

    def cooked_package(self):
        package = pathlib.Path(self.temp.name) / "package"
        package.mkdir(exist_ok=True)
        (package / "BUILD_SHA.txt").write_text(self.base, encoding="utf-8")
        (package / "GATE_MANIFEST.txt").write_text(
            f"source_sha={self.base}\nbase_cooked_sha={self.base}\n"
            "content_changed_since_base=0\nconfig_changed_since_base=0\n", encoding="utf-8")
        return package

    def overlay(self, package):
        return self.execute(marked_script(WORKFLOWS / "pinkcab-g1-github-control-plane.yml",
                                          "PINKCAB_COOKED_SCOPE"), ACCEPTED_BASE=str(package))

    def test_overlay_rejects_earlier_config_change_after_later_docs_commit(self):
        package = self.cooked_package()
        self.write("Config/DefaultEngine.ini", "changed config requiring cook")
        self.commit("config change")
        self.write("docs/readme.md", "documentation follow-up")
        self.commit("docs change")
        result = self.overlay(package)
        self.assertNotEqual(result.returncode, 0, result.stdout)
        self.assertIn("PINKCAB_RECOOK_REQUIRED", result.stdout)

    def test_overlay_allows_code_only_changes_and_rejects_missing_provenance(self):
        package = self.cooked_package()
        self.write("Source/PinkCab/Private/Changed.cpp", "// code only")
        self.commit("code change")
        result = self.overlay(package)
        self.assertEqual(result.returncode, 0, result.stdout)
        (package / "GATE_MANIFEST.txt").unlink()
        result = self.overlay(package)
        self.assertNotEqual(result.returncode, 0, result.stdout)
        self.assertIn("PINKCAB_COOKED_PROVENANCE_MISSING", result.stdout)


if __name__ == "__main__":
    unittest.main()
