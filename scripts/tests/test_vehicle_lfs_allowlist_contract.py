import json
import unittest
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
CONFIG = REPO / "Config" / "VehiclePhysicsRuntimeFixtures.json"
WORKFLOWS = (
    REPO / ".github" / "workflows" / "pinkcab-vehicle-physics-tdd.yml",
    REPO / ".github" / "workflows" / "cd648-p02-phy009.yml",
    REPO / ".github" / "workflows" / "cd869-deliver.yml",
)

EXPECTED = {
    "Content/Dev/Maps/L_PinkCab_L1_EndlessStraight.umap",
    "Content/Dev/Vehicles/Definitions/DA_PC_Vehicle_fixture.uasset",
    "Content/Dev/Vehicles/Definitions/DA_PC_Vehicle_tatra613.uasset",
    "Content/Dev/Vehicles/Tatra613Presentation/Materials/M_PC_Tatra_BodyPaint.uasset",
    "Content/Dev/Vehicles/Tatra613Presentation/Materials/M_PC_Tatra_Chrome.uasset",
    "Content/Dev/Vehicles/Tatra613Presentation/Materials/M_PC_Tatra_Fabric.uasset",
    "Content/Dev/Vehicles/Tatra613Presentation/Materials/M_PC_Tatra_Glass.uasset",
    "Content/Dev/Vehicles/Tatra613Presentation/Materials/M_PC_Tatra_InteriorVinyl.uasset",
    "Content/Dev/Vehicles/Tatra613Presentation/Materials/M_PC_Tatra_LightAmber.uasset",
    "Content/Dev/Vehicles/Tatra613Presentation/Materials/M_PC_Tatra_LightRed.uasset",
    "Content/Dev/Vehicles/Tatra613Presentation/Materials/M_PC_Tatra_LightWhite.uasset",
    "Content/Dev/Vehicles/Tatra613Presentation/Materials/M_PC_Tatra_Mirror.uasset",
    "Content/Dev/Vehicles/Tatra613Presentation/Materials/M_PC_Tatra_RubberPlastic.uasset",
    "Content/Dev/Vehicles/Tatra613Presentation/Materials/T_TatraMicroNoise.uasset",
}


class VehicleLfsAllowlistContractTests(unittest.TestCase):
    def test_allowlist_is_exact_and_complete(self):
        data = json.loads(CONFIG.read_text(encoding="utf-8"))
        allow = data["allow_changed_lfs"]
        self.assertEqual(set(allow), EXPECTED)
        self.assertEqual(len(allow), len(EXPECTED))
        for path in allow:
            self.assertFalse(any(token in path for token in ("*", "?", "[", "]")))
            self.assertTrue(path.startswith("Content/"))

    def test_all_runtime_workflows_consume_central_allowlist(self):
        for workflow in WORKFLOWS:
            source = workflow.read_text(encoding="utf-8")
            with self.subTest(workflow=workflow.name):
                self.assertIn("allow_changed_lfs", source)
                self.assertIn("$materializeArgs += '--allow-changed-lfs'", source)
                self.assertNotIn(
                    "'--allow-changed-lfs', 'Content/Dev/Maps/L_PinkCab_L1_EndlessStraight.umap'",
                    source,
                )
                self.assertNotIn(
                    '--allow-changed-lfs "Content/Dev/Maps/L_PinkCab_L1_EndlessStraight.umap"',
                    source,
                )


if __name__ == "__main__":
    unittest.main()
