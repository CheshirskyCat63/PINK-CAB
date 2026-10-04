import unittest
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
PROFILE_CPP = REPO / "Source" / "PinkCab" / "Private" / "Runtime" / "PinkCabVehicleVisualProfile.cpp"
PROFILE_H = REPO / "Source" / "PinkCab" / "Public" / "Runtime" / "PinkCabVehicleVisualProfile.h"
PAWN_CPP = REPO / "Source" / "PinkCab" / "Private" / "Runtime" / "PinkCabChaosTatraPawn.cpp"
VISUAL_CPP = REPO / "Source" / "PinkCab" / "Private" / "Runtime" / "PinkCabChaosTatraPawnVisual.cpp"
DEFINITION_CPP = REPO / "Source" / "PinkCab" / "Private" / "Runtime" / "PinkCabChaosTatraPawnVehicleDefinition.cpp"


class GenericVehicleRuntimeTests(unittest.TestCase):
    def test_visual_profile_runtime_has_no_tatra_asset_hardcode(self):
        source = PROFILE_CPP.read_text(encoding="utf-8")
        for forbidden in (
            "Tatra613ScenePreserved",
            "Tatra613DesktopScene",
            "Tatra613Presentation",
            "PinkCabTatra613SceneMeshPaths",
            "pessimat613_",
            "t613_",
            "HeroMaterialForMeshPath",
        ):
            self.assertNotIn(forbidden, source)

    def test_visual_profile_header_has_no_vehicle_factory(self):
        source = PROFILE_H.read_text(encoding="utf-8")
        self.assertNotIn("Tatra613ScenePreserved", source)

    def test_begin_play_uses_vehicle_settings_not_tatra_factory(self):
        source = PAWN_CPP.read_text(encoding="utf-8")
        self.assertIn("UPinkCabVehicleSettings", source)
        self.assertIn("LoadSelectedDefinition", source)
        self.assertNotIn("Tatra613ScenePreserved", source)

    def test_steering_uses_authored_profile_anchor(self):
        source = VISUAL_CPP.read_text(encoding="utf-8")
        self.assertIn("SteeringPresentationPivot", source)
        self.assertIn("SteeringPresentationAxis", source)
        self.assertNotIn("GetBounds()", source)

    def test_visual_wheel_parts_come_from_definition_bindings(self):
        visual = VISUAL_CPP.read_text(encoding="utf-8")
        apply_source = DEFINITION_CPP.read_text(encoding="utf-8")
        self.assertIn("ActiveWheelPresentationPartIds", visual)
        self.assertIn("PresentationPartId", apply_source)
        for hardcoded in ('TEXT("WheelFL")', 'TEXT("WheelFR")', 'TEXT("WheelRL")', 'TEXT("WheelRR")'):
            self.assertNotIn(hardcoded, visual)


if __name__ == "__main__":
    unittest.main()
