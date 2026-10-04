import pathlib
import unittest


ROOT = pathlib.Path(__file__).resolve().parents[2]
HARNESS = ROOT / "Source" / "PinkCab" / "Private" / "Runtime" / "PinkCabChaosTatraPawnP04Acceptance.cpp"
PHASES = ROOT / "Source" / "PinkCab" / "Private" / "Runtime" / "PinkCabChaosTatraPawnP04AcceptancePhases.cpp"
SCRIPT = ROOT / "scripts" / "ci" / "run-p04-packaged-acceptance.ps1"
DELIVERY = ROOT / ".github" / "workflows" / "cd869-deliver.yml"
P02 = ROOT / ".github" / "workflows" / "cd648-p02-phy009.yml"


class P04PackagedAcceptanceContractTests(unittest.TestCase):
    def setUp(self):
        self.harness = HARNESS.read_text(encoding="utf-8") + "\n" + PHASES.read_text(encoding="utf-8")
        self.script = SCRIPT.read_text(encoding="utf-8")
        self.delivery = DELIVERY.read_text(encoding="utf-8")
        self.p02 = P02.read_text(encoding="utf-8")

    def test_harness_is_opt_in_and_uses_the_player_control_runtime(self):
        self.assertIn("PinkCabP04Acceptance=", self.harness)
        self.assertIn("ApplyVehicleInputFrame(", self.harness)
        self.assertIn("CockpitState.SetSelectedGear(", self.harness)
        self.assertIn("VehicleControlRuntime.ResetHandbrake(", self.harness)
        self.assertIn("source=PACKAGED_RUNTIME_CONTROL_PATH", self.harness)

    def test_harness_does_not_inject_force_velocity_or_force_gear(self):
        forbidden = (
            "AddForce(",
            "AddImpulse(",
            "SetPhysicsLinearVelocity(",
            "SetActorLocation(",
            "SetActorTransform(",
            "ForceGearState(",
        )
        for token in forbidden:
            with self.subTest(token=token):
                self.assertNotIn(token, self.harness)

    def test_runtime_ab_override_is_bounded_to_the_historical_first_ratio(self):
        self.assertIn('CaseName == TEXT("forward_baseline")', self.harness)
        self.assertIn("ForwardGearRatios[0] = 4.60f", self.harness)
        self.assertIn("profile_asset_mutated=0 transient_ab_ratio_override=%d force_injection=0", self.harness)

    def test_packaged_matrix_covers_remaining_p04_and_countersteer_gate(self):
        for case in (
            "forward_baseline",
            "forward_candidate",
            "reverse25",
            "reverse50",
            "reverse100",
            "lift",
            "counter_low",
            "counter_urban",
            "counter_high",
            "top",
        ):
            with self.subTest(case=case):
                self.assertIn(f"'{case}'", self.script)
        for marker in (
            "PINKCAB_P04_PHY017_AB=PASS",
            "PINKCAB_P04_PHY019_REVERSE=PASS",
            "PINKCAB_P04_T026_COUNTERSTEER=PASS",
            "PINKCAB_P04_PHY018_TOP=PASS",
            "PINKCAB_P04_PHY036_NO_ANTISTALL=PASS",
            "PINKCAB_P04_PACKAGED_ACCEPTANCE=PASS",
        ):
            with self.subTest(marker=marker):
                self.assertIn(marker, self.script)

    def test_delivery_runs_packaged_acceptance_before_install(self):
        acceptance = self.delivery.index("- name: P04 packaged road acceptance")
        install = self.delivery.index("- name: Install and verify immutable candidate")
        self.assertLess(acceptance, install)
        self.assertIn("run-p04-packaged-acceptance.ps1", self.delivery)

    def test_delivery_cooks_and_proves_the_real_tatra_visual_shell(self):
        for token in (
            "Content\\Dev\\Vehicles\\Tatra613DesktopScene",
            "Content\\Dev\\Vehicles\\Tatra613ArchiveV12Clean",
            "CD869_REQUIRED_COOK_DIR_MISSING",
            "PINKCAB_TATRA_VISUAL=PASS .*physics_chassis_hidden=1",
            "CD869_SMOKE_TATRA_VISUAL_MISSING",
        ):
            with self.subTest(token=token):
                self.assertIn(token, self.delivery)
        pawn = (ROOT / "Source" / "PinkCab" / "Private" / "Runtime" / "PinkCabChaosTatraPawn.cpp").read_text(encoding="utf-8")
        self.assertIn("PINKCAB_TATRA_VISUAL=PASS", pawn)
        self.assertIn("PINKCAB_TATRA_VISUAL=FAIL", pawn)
        self.assertIn("Tatra613ScenePreserved()", pawn)

    def test_frozen_p02_scope_admits_only_the_explicit_new_acceptance_files(self):
        for path in (
            "Source/PinkCab/Private/Runtime/PinkCabChaosTatraPawnP04Acceptance.cpp",
            "Source/PinkCab/Private/Runtime/PinkCabChaosTatraPawnP04AcceptancePhases.cpp",
            "scripts/ci/run-p04-packaged-acceptance.ps1",
            "scripts/tests/test_p04_packaged_acceptance_contract.py",
        ):
            with self.subTest(path=path):
                self.assertIn(f"'{path}'", self.p02)


if __name__ == "__main__":
    unittest.main()
