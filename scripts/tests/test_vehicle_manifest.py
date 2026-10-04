import copy
import json
import sys
import unittest
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
VEHICLE_SCRIPTS = REPO / "scripts" / "vehicles"
sys.path.insert(0, str(VEHICLE_SCRIPTS))

from vehicle_manifest import ManifestError, manifest_digest, normalize_manifest, validate_manifest


def transform(location=None, rotation=None, scale=None):
    return {
        "location": location or [0.0, 0.0, 0.0],
        "rotation": rotation or [0.0, 0.0, 0.0],
        "scale": scale or [1.0, 1.0, 1.0],
    }


def valid_manifest():
    wheels = []
    parts = []
    for wheel_id, bone in (
        ("WheelFL", "Phys_Wheel_FL"),
        ("WheelFR", "Phys_Wheel_FR"),
        ("WheelRL", "Phys_Wheel_BL"),
        ("WheelRR", "Phys_Wheel_BR"),
    ):
        wheels.append(
            {
                "id": wheel_id,
                "bone": bone,
                "presentation_part": wheel_id,
                "mesh": "/Engine/BasicShapes/Cylinder.Cylinder",
                "transform": transform(),
            }
        )
        parts.append(
            {
                "id": wheel_id,
                "mesh": "/Engine/BasicShapes/Cylinder.Cylinder",
                "transform": transform(),
                "material_semantic": "RubberPlastic",
            }
        )

    return {
        "schema_version": 1,
        "slug": "fixture",
        "vehicle_id": "PinkCab.Vehicle.Fixture",
        "definition_asset": "/Game/Dev/Vehicles/Definitions/DA_PC_Vehicle_fixture",
        "source": {
            "path": "SourceAssets/Vehicles/fixture/fixture.glb",
            "provenance": "test-fixture",
            "mode": "existing_assets",
        },
        "physics": {
            "carrier_mesh": "/Game/Vehicles/SportsCar/SKM_SportsCar.SKM_SportsCar",
            "physics_asset": "/Game/Vehicles/SportsCar/PA_SportsCar.PA_SportsCar",
            "wheels": wheels,
        },
        "presentation": {
            "mode": "explicit",
            "asset_root": "/Engine/BasicShapes",
            "root_transform": transform(),
            "parts": parts,
            "material_palette": {
                "BodyPaint": "/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial",
                "RubberPlastic": "/Engine/EngineMaterials/DefaultMaterial.DefaultMaterial",
            },
            "material_rules": [],
            "articulations": [
                {
                    "id": "DoorFL",
                    "pivot": [50.0, -80.0, 70.0],
                    "axis": [0.0, 0.0, 1.0],
                    "open_angle_degrees": 70.0,
                    "travel_seconds": 0.5,
                    "parts": ["WheelFL"],
                }
            ],
            "steering": None,
            "cockpit_bindings": [],
        },
        "driver": {
            "mesh": "/Game/Characters/Mannequins/Meshes/SKM_Manny_Simple.SKM_Manny_Simple",
            "transform": transform([-30.0, -38.0, -55.0], scale=[0.92, 0.92, 0.92]),
            "head_transform": transform([-18.0, -40.0, 112.0]),
        },
    }


class VehicleManifestTests(unittest.TestCase):
    def test_valid_manifest_normalizes(self):
        normalized = normalize_manifest(valid_manifest())
        self.assertEqual(normalized["schema_version"], 1)
        self.assertEqual(normalized["slug"], "fixture")
        self.assertEqual(len(normalized["physics"]["wheels"]), 4)
        self.assertEqual(validate_manifest(normalized), normalized)

    def test_missing_required_field_fails(self):
        manifest = valid_manifest()
        del manifest["physics"]["carrier_mesh"]
        with self.assertRaisesRegex(ManifestError, "carrier_mesh"):
            validate_manifest(manifest)

    def test_wheel_semantics_must_be_unique_and_complete(self):
        manifest = valid_manifest()
        manifest["physics"]["wheels"][3]["id"] = "WheelFL"
        with self.assertRaisesRegex(ManifestError, "wheel"):
            validate_manifest(manifest)

    def test_articulation_axis_must_be_nonzero(self):
        manifest = valid_manifest()
        manifest["presentation"]["articulations"][0]["axis"] = [0.0, 0.0, 0.0]
        with self.assertRaisesRegex(ManifestError, "axis"):
            validate_manifest(manifest)

    def test_part_cannot_belong_to_two_articulations(self):
        manifest = valid_manifest()
        second = copy.deepcopy(manifest["presentation"]["articulations"][0])
        second["id"] = "DoorOther"
        manifest["presentation"]["articulations"].append(second)
        with self.assertRaisesRegex(ManifestError, "articulation"):
            validate_manifest(manifest)

    def test_unknown_material_semantic_fails(self):
        manifest = valid_manifest()
        manifest["presentation"]["parts"][0]["material_semantic"] = "MagicPaint"
        with self.assertRaisesRegex(ManifestError, "material semantic"):
            validate_manifest(manifest)

    def test_digest_is_deterministic_for_key_order(self):
        manifest = valid_manifest()
        reordered = json.loads(
            json.dumps(manifest, sort_keys=True),
            object_pairs_hook=lambda pairs: dict(reversed(pairs)),
        )
        self.assertEqual(manifest_digest(manifest), manifest_digest(reordered))

    def test_repository_manifests_validate(self):
        for name in ("tatra613.vehicle.json", "fixture.vehicle.json"):
            path = REPO / "Config" / "Vehicles" / name
            data = json.loads(path.read_text(encoding="utf-8"))
            normalized = validate_manifest(data)
            self.assertEqual(normalized["schema_version"], 1)


if __name__ == "__main__":
    unittest.main()
