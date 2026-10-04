from __future__ import annotations

import json
import os
import sys
from pathlib import Path
from typing import Any

import unreal

SCRIPT_DIR = Path(__file__).resolve().parent
if str(SCRIPT_DIR) not in sys.path:
    sys.path.insert(0, str(SCRIPT_DIR))

from vehicle_manifest import load_manifest, manifest_digest

ASSET_TOOLS = unreal.AssetToolsHelpers.get_asset_tools()


def load_required_asset(path: str):
    asset = unreal.load_asset(path)
    if asset is None:
        raise RuntimeError(f"required asset does not resolve: {path}")
    return asset


def _asset_leaf(path: str) -> str:
    package = path.split(".", 1)[0]
    return package.rsplit("/", 1)[-1]


def _name(value: str):
    return unreal.Name(value)


def _vector(values: list[float]):
    return unreal.Vector(float(values[0]), float(values[1]), float(values[2]))


def _transform(data: dict[str, Any]):
    result = unreal.Transform()
    result.set_editor_property("translation", _vector(data["location"]))
    rot = unreal.Rotator()
    rot.set_editor_property("pitch", float(data["rotation"][0]))
    rot.set_editor_property("yaw", float(data["rotation"][1]))
    rot.set_editor_property("roll", float(data["rotation"][2]))
    result.set_editor_property("rotation", rot.quaternion())
    result.set_editor_property("scale3d", _vector(data["scale"]))
    return result


def _material_for_leaf(
    leaf: str,
    palette: dict[str, str],
    rules: list[dict[str, Any]],
):
    lowered = leaf.lower()
    for rule in rules:
        if any(token.lower() in lowered for token in rule["contains"]):
            return load_required_asset(palette[rule["semantic"]])
    return None


def _new_part(
    part_id: str,
    mesh_path: str,
    transform: dict[str, Any],
    material=None,
):
    part = unreal.PinkCabVehiclePresentationPart()
    part.set_editor_property("part_id", _name(part_id))
    part.set_editor_property("mesh", load_required_asset(mesh_path))
    part.set_editor_property("local_transform", _transform(transform))
    if material is not None:
        part.set_editor_property("material_override", material)
    return part


def _discover_parts(manifest: dict[str, Any]):
    presentation = manifest["presentation"]
    root = presentation["asset_root"]
    excluded = set(presentation["discovery"]["exclude_leafs"])
    paths = []
    for path in unreal.EditorAssetLibrary.list_assets(
        root, recursive=True, include_folder=False
    ):
        asset = unreal.load_asset(path)
        if asset is None or asset.get_class().get_name() != "StaticMesh":
            continue
        leaf = _asset_leaf(path)
        if leaf in excluded:
            continue
        paths.append(str(path))
    paths.sort()

    id_format = presentation["discovery"]["id_format"]
    root_transform = presentation["root_transform"]
    palette = presentation["material_palette"]
    rules = presentation["material_rules"]
    parts = []
    leaf_to_part_id: dict[str, str] = {}
    for index, path in enumerate(paths):
        leaf = _asset_leaf(path)
        part_id = id_format.format(index=index)
        if leaf in leaf_to_part_id:
            raise RuntimeError(f"duplicate discovered leaf: {leaf}")
        leaf_to_part_id[leaf] = part_id
        parts.append(
            _new_part(
                part_id,
                path,
                root_transform,
                _material_for_leaf(leaf, palette, rules),
            )
        )
    if not parts:
        raise RuntimeError(f"no StaticMesh assets discovered under {root}")
    return parts, leaf_to_part_id


def _explicit_parts(manifest: dict[str, Any]):
    presentation = manifest["presentation"]
    palette = presentation["material_palette"]
    parts = []
    ids: dict[str, str] = {}
    for source in presentation["parts"]:
        material = None
        semantic = source["material_semantic"]
        if semantic:
            material = load_required_asset(palette[semantic])
        parts.append(
            _new_part(
                source["id"],
                source["mesh"],
                source["transform"],
                material,
            )
        )
        ids[source["id"]] = source["id"]
    return parts, ids


def _ensure_wheel_parts(
    manifest: dict[str, Any],
    parts: list,
    id_lookup: dict[str, str],
):
    existing = {
        str(part.get_editor_property("part_id"))
        for part in parts
    }
    for wheel in manifest["physics"]["wheels"]:
        part_id = wheel["presentation_part"]
        if part_id not in existing:
            parts.append(
                _new_part(
                    part_id,
                    wheel["mesh"],
                    wheel["transform"],
                    None,
                )
            )
            existing.add(part_id)
        id_lookup.setdefault(part_id, part_id)


def _articulations(manifest: dict[str, Any], id_lookup: dict[str, str]):
    result = []
    for source in manifest["presentation"]["articulations"]:
        member_ids = []
        for authored_id in source["parts"]:
            resolved = id_lookup.get(authored_id)
            if not resolved:
                raise RuntimeError(
                    f"articulation {source['id']} references unknown part {authored_id}"
                )
            member_ids.append(_name(resolved))
        item = unreal.PinkCabVehicleArticulationDefinition()
        item.set_editor_property("articulation_id", _name(source["id"]))
        item.set_editor_property("pivot_local", _vector(source["pivot"]))
        item.set_editor_property("axis_local", _vector(source["axis"]))
        item.set_editor_property(
            "open_angle_degrees", float(source["open_angle_degrees"])
        )
        item.set_editor_property("travel_seconds", float(source["travel_seconds"]))
        item.set_editor_property("part_ids", member_ids)
        result.append(item)
    return result


def _wheel_bindings(manifest: dict[str, Any]):
    result = []
    for source in manifest["physics"]["wheels"]:
        item = unreal.PinkCabVehicleWheelBinding()
        item.set_editor_property("wheel_id", _name(source["id"]))
        item.set_editor_property("bone_name", _name(source["bone"]))
        item.set_editor_property(
            "presentation_part_id", _name(source["presentation_part"])
        )
        result.append(item)
    return result


def _resolve_source(manifest: dict[str, Any]) -> str:
    source = manifest["source"]
    root_kind = source.get("root")
    raw = Path(source["path"])
    if raw.is_absolute():
        resolved = raw
    elif root_kind == "studio":
        studio = os.environ.get("PINKCAB_STUDIO_ROOT")
        if not studio:
            raise RuntimeError("PINKCAB_STUDIO_ROOT is required for source.root=studio")
        resolved = Path(studio) / raw
    else:
        resolved = Path(unreal.Paths.project_dir()) / raw
    if root_kind is not None and not resolved.exists():
        raise RuntimeError(f"canonical source does not exist: {resolved}")
    return str(resolved.resolve()) if resolved.exists() else str(resolved)


def _profile_from_manifest(manifest: dict[str, Any]):
    presentation = manifest["presentation"]
    if presentation["mode"] == "discover":
        parts, id_lookup = _discover_parts(manifest)
    else:
        parts, id_lookup = _explicit_parts(manifest)

    _ensure_wheel_parts(manifest, parts, id_lookup)

    profile = unreal.PinkCabVehicleVisualProfile()
    profile.set_editor_property(
        "profile_id", _name(f"PinkCab.Visual.{manifest['slug']}")
    )
    profile.set_editor_property("presentation_parts", parts)
    profile.set_editor_property(
        "articulations", _articulations(manifest, id_lookup)
    )
    profile.set_editor_property(
        "driver_head_transform", _transform(manifest["driver"]["head_transform"])
    )

    steering = presentation["steering"]
    if steering:
        resolved = id_lookup.get(steering["part"])
        if not resolved:
            raise RuntimeError(
                f"steering references unknown part {steering['part']}"
            )
        profile.set_editor_property(
            "steering_presentation_part_id", _name(resolved)
        )
        profile.set_editor_property(
            "steering_presentation_pivot", _vector(steering["pivot"])
        )
        profile.set_editor_property(
            "steering_presentation_axis", _vector(steering["axis"])
        )

    if presentation["cockpit_bindings"]:
        raise RuntimeError(
            "cockpit_bindings authoring is not implemented; refuse partial definition"
        )
    return profile


def _definition_class():
    cls = getattr(unreal, "PinkCabVehicleDefinition", None)
    if cls is None:
        raise RuntimeError("PinkCabVehicleDefinition is not exposed to Unreal Python")
    return cls


def _split_asset_path(asset_path: str):
    package = asset_path.split(".", 1)[0]
    folder, name = package.rsplit("/", 1)
    return folder, name


def _load_or_create_definition(asset_path: str):
    cls = _definition_class()
    definition = (
        unreal.load_asset(asset_path)
        if unreal.EditorAssetLibrary.does_asset_exist(asset_path)
        else None
    )
    if definition is not None:
        if definition.get_class().get_name() != "PinkCabVehicleDefinition":
            raise RuntimeError(
                f"definition path is occupied by {definition.get_class().get_name()}: {asset_path}"
            )
        return definition

    folder, name = _split_asset_path(asset_path)
    unreal.EditorAssetLibrary.make_directory(folder)
    factory = unreal.DataAssetFactory()
    factory.set_editor_property("data_asset_class", cls)
    definition = ASSET_TOOLS.create_asset(name, folder, cls, factory)
    if definition is None:
        raise RuntimeError(f"create_asset failed: {asset_path}")
    return definition


def validate_definition_fields(definition, manifest: dict[str, Any]) -> None:
    if str(definition.get_editor_property("vehicle_id")) != manifest["vehicle_id"]:
        raise RuntimeError("definition vehicle_id readback mismatch")
    wheels = definition.get_editor_property("wheels")
    if len(wheels) != 4:
        raise RuntimeError("definition wheel readback mismatch")
    profile = definition.get_editor_property("visual_profile")
    if len(profile.get_editor_property("presentation_parts")) < 4:
        raise RuntimeError("definition presentation readback is incomplete")
    if len(profile.get_editor_property("articulations")) != len(
        manifest["presentation"]["articulations"]
    ):
        raise RuntimeError("definition articulation readback mismatch")

    for required in (
        manifest["physics"]["carrier_mesh"],
        manifest["physics"]["physics_asset"],
        manifest["driver"]["mesh"],
    ):
        load_required_asset(required)


def author_vehicle(manifest_path: str, receipt_path: str) -> dict[str, Any]:
    manifest = load_manifest(manifest_path)
    digest = manifest_digest(manifest)
    resolved_source = _resolve_source(manifest)

    profile = _profile_from_manifest(manifest)
    definition = _load_or_create_definition(manifest["definition_asset"])
    definition.set_editor_property("vehicle_id", _name(manifest["vehicle_id"]))
    definition.set_editor_property(
        "physics_carrier_mesh",
        load_required_asset(manifest["physics"]["carrier_mesh"]),
    )
    definition.set_editor_property(
        "physics_asset",
        load_required_asset(manifest["physics"]["physics_asset"]),
    )
    definition.set_editor_property("wheels", _wheel_bindings(manifest))
    definition.set_editor_property("visual_profile", profile)
    definition.set_editor_property(
        "driver_mesh", load_required_asset(manifest["driver"]["mesh"])
    )
    definition.set_editor_property(
        "driver_transform", _transform(manifest["driver"]["transform"])
    )
    definition.set_editor_property(
        "driver_head_transform", _transform(manifest["driver"]["head_transform"])
    )

    validate_definition_fields(definition, manifest)
    if not unreal.EditorAssetLibrary.save_loaded_asset(
        definition, only_if_is_dirty=False
    ):
        raise RuntimeError("save_loaded_asset failed for vehicle definition")

    saved = unreal.load_asset(manifest["definition_asset"])
    if saved is None:
        raise RuntimeError("saved definition cannot be loaded back")
    validate_definition_fields(saved, manifest)

    saved_profile = saved.get_editor_property("visual_profile")
    parts = saved_profile.get_editor_property("presentation_parts")
    material_overrides = sum(
        1
        for part in parts
        if part.get_editor_property("material_override") is not None
    )
    receipt = {
        "manifest_digest": digest,
        "definition_asset": manifest["definition_asset"],
        "vehicle_id": manifest["vehicle_id"],
        "slug": manifest["slug"],
        "source": resolved_source,
        "presentation_part_count": len(parts),
        "material_override_count": material_overrides,
        "articulation_count": len(
            saved_profile.get_editor_property("articulations")
        ),
        "wheel_count": len(saved.get_editor_property("wheels")),
    }

    out = Path(receipt_path)
    out.parent.mkdir(parents=True, exist_ok=True)
    payload = json.dumps(receipt, indent=2, sort_keys=True) + "\n"
    out.write_text(payload, encoding="utf-8")
    unreal.log(
        "PINKCAB_VEHICLE_AUTHOR=PASS "
        f"vehicle={manifest['vehicle_id']} parts={len(parts)} "
        f"articulations={receipt['articulation_count']} digest={digest}"
    )
    return receipt


def main() -> None:
    manifest_path = os.environ.get("PINKCAB_VEHICLE_MANIFEST")
    receipt_path = os.environ.get("PINKCAB_VEHICLE_RECEIPT")
    if not manifest_path or not receipt_path:
        raise RuntimeError(
            "PINKCAB_VEHICLE_MANIFEST and PINKCAB_VEHICLE_RECEIPT are required"
        )
    author_vehicle(manifest_path, receipt_path)


if __name__ == "__main__":
    main()
