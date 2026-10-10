"""Keep known Task6 blockers in the existing authoritative runtime gate."""

from pathlib import Path
import re
import unittest


class VehicleRuntimeSelection(unittest.TestCase):
    def test_partial_clutch_blockers_are_executed_not_only_documented(self):
        script = Path(__file__).resolve().parents[1] / "verify-runtime.ps1"
        source = script.read_text(encoding="utf-8-sig")
        match = re.search(r"\$tests\s*=\s*@\((.*?)\n\)", source, re.DOTALL)
        self.assertIsNotNone(match, "explicit canonical test list is required")
        entries = re.findall(r"^\s*'([^']+)'", match.group(1), re.MULTILINE)
        self.assertEqual(len(entries), len(set(entries)), "duplicate test names")
        for required in (
            "PinkCab.Vehicle.Actuation.PartialClutchTransfer",
            "PinkCab.Vehicle.Input.ClutchCapability",
            "PinkCab.Vehicle.Actuation.NativeJointCapacity",
            "PinkCab.Vehicle.Actuation.ClutchCommandWake",
            "PinkCab.Vehicle.Actuation.ClutchBackDrivePower",
            "PinkCab.Vehicle.Actuation.ClutchEnvelopeRuntime",
            "PinkCab.Vehicle.Actuation.ClutchLoadedLock",
        ):
            with self.subTest(required=required):
                self.assertIn(required, entries, "known Task6 failure must block the PR")



class RetiredClutchImplementation(unittest.TestCase):
    def test_retired_solver_is_not_compiled_beside_native_chaos(self):
        root = Path(__file__).resolve().parents[2]
        for relative in (
            "Source/PinkCabVehicle/Private/Vehicle/PinkCabClutchDrivelineIntegration.cpp",
            "Source/PinkCabVehicle/Private/Vehicle/PinkCabClutchDrivelineIntegration.h",
            "Source/PinkCabVehicle/Private/Vehicle/PinkCabClutchDrivelineModel.cpp",
            "Source/PinkCabVehicle/Public/Vehicle/PinkCabClutchDrivelineModel.h",
        ):
            with self.subTest(path=relative):
                self.assertFalse((root / relative).exists(), "retired solver belongs in Git history")
        self.assertTrue((root / "Source/PinkCabVehicle/Public/Vehicle/PinkCabClutchDrivelineConfig.h").exists(),
                        "versioned calibration data must be retained separately")


if __name__ == "__main__":
    unittest.main()
