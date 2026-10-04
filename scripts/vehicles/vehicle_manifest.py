from __future__ import annotations

import copy
import hashlib
import json
import math
import re
from pathlib import Path
from typing import Any

SCHEMA_VERSION = 1
WHEEL_IDS = ("WheelFL", "WheelFR", "WheelRL", "WheelRR")
MATERIAL_SEMANTICS = frozenset(
    {
        "BodyPaint",
        "Chrome",
        "Glass",
        "RubberPlastic",
        "InteriorVinyl",
        "Fabric",
        "Mirror",
        "LightWhite",
        "LightRed",
        "LightAmber",
    }
)
SLUG_RE = re.compile(r"^[a-z0-9][a-z0-9_-]*$")


class ManifestError(ValueError):
    pass


def _fail(path: str, message: str) -> None:
    raise ManifestError(f"{path}: {message}")


def _mapping(value: Any, path: str) -> dict[str, Any]:
    if not isinstance(value, dict):
        _fail(path, "must be an object")
    return value


def _string(value: Any, path: str) -> str:
    if not isinstance(value, str) or not value.strip():
        _fail(path, "must be a non-empty string")
    return value.strip()


def _triple(value: Any, path: str, *, nonzero: bool = False) -> list[float]:
    if not isinstance(value, list) or len(value) != 3:
        _fail(path, "must contain exactly three numbers")
    result: list[float] = []
    for index, item in enumerate(value):
        if isinstance(item, bool) or not isinstance(item, (int, float)):
            _fail(f"{path}[{index}]", "must be numeric")
        number = float(item)
        if not math.isfinite(number):
            _fail(f"{path}[{index}]", "must be finite")
        result.append(number)
    if nonzero and sum(component * component for component in result) <= 1.0e-12:
        _fail(path, "axis must be nonzero")
    return result


def _transform(value: Any, path: str) -> dict[str, list[float]]:
    source = _mapping(value, path)
    location = _triple(source.get("location"), f"{path}.location")
    rotation = _triple(source.get("rotation"), f"{path}.rotation")
    scale = _triple(source.get("scale"), f"{path}.scale")
    if any(component <= 0.0 for component in scale):
        _fail(f"{path}.scale", "components must be positive")
    return {"location": location, "rotation": rotation, "scale": scale}


def _asset_path(value: Any, path: str) -> str:
    result = _string(value, path)
    if not result.startswith(("/Game/", "/Engine/")):
        _fail(path, "must be an Unreal /Game or /Engine asset path")
    return result


def _unique(values: list[str], path: str) -> None:
    if len(values) != len(set(values)):
        _fail(path, "values must be unique")


def _validate_materials(
    presentation: dict[str, Any],
) -> tuple[dict[str, str], list[dict[str, Any]]]:
    raw_palette = _mapping(
        presentation.get("material_palette", {}), "presentation.material_palette"
    )
    palette: dict[str, str] = {}
    for semantic, raw_path in raw_palette.items():
        if semantic not in MATERIAL_SEMANTICS:
            _fail(
                f"presentation.material_palette.{semantic}",
                f"unknown material semantic {semantic}",
            )
        palette[semantic] = _asset_path(
            raw_path, f"presentation.material_palette.{semantic}"
        )

    raw_rules = presentation.get("material_rules", [])
    if not isinstance(raw_rules, list):
        _fail("presentation.material_rules", "must be an array")
    rules: list[dict[str, Any]] = []
    for index, raw in enumerate(raw_rules):
        path = f"presentation.material_rules[{index}]"
        rule = _mapping(raw, path)
        semantic = _string(rule.get("semantic"), f"{path}.semantic")
        if semantic not in MATERIAL_SEMANTICS:
            _fail(f"{path}.semantic", f"unknown material semantic {semantic}")
        if semantic not in palette:
            _fail(f"{path}.semantic", f"material semantic {semantic} has no palette entry")
        contains = rule.get("contains")
        if not isinstance(contains, list) or not contains:
            _fail(f"{path}.contains", "must contain at least one substring")
        tokens = [_string(token, f"{path}.contains") for token in contains]
        rules.append({"semantic": semantic, "contains": tokens})
    return palette, rules


def _validate_parts(
    presentation: dict[str, Any],
    palette: dict[str, str],
) -> tuple[list[dict[str, Any]], set[str]]:
    raw_parts = presentation.get("parts", [])
    if not isinstance(raw_parts, list):
        _fail("presentation.parts", "must be an array")
    parts: list[dict[str, Any]] = []
    ids: list[str] = []
    for index, raw in enumerate(raw_parts):
        path = f"presentation.parts[{index}]"
        part = _mapping(raw, path)
        part_id = _string(part.get("id"), f"{path}.id")
        semantic = part.get("material_semantic")
        if semantic is not None:
            semantic = _string(semantic, f"{path}.material_semantic")
            if semantic not in MATERIAL_SEMANTICS:
                _fail(f"{path}.material_semantic", f"unknown material semantic {semantic}")
            if semantic not in palette:
                _fail(f"{path}.material_semantic", f"material semantic {semantic} has no palette entry")
        parts.append(
            {
                "id": part_id,
                "mesh": _asset_path(part.get("mesh"), f"{path}.mesh"),
                "transform": _transform(part.get("transform"), f"{path}.transform"),
                "material_semantic": semantic,
            }
        )
        ids.append(part_id)
    _unique(ids, "presentation.parts")
    return parts, set(ids)


def _validate_wheels(physics: dict[str, Any]) -> list[dict[str, Any]]:
    raw_wheels = physics.get("wheels")
    if not isinstance(raw_wheels, list) or len(raw_wheels) != 4:
        _fail("physics.wheels", "exactly four wheel bindings are required")
    result: list[dict[str, Any]] = []
    ids: list[str] = []
    bones: list[str] = []
    part_ids: list[str] = []
    for index, raw in enumerate(raw_wheels):
        path = f"physics.wheels[{index}]"
        wheel = _mapping(raw, path)
        wheel_id = _string(wheel.get("id"), f"{path}.id")
        bone = _string(wheel.get("bone"), f"{path}.bone")
        part_id = _string(wheel.get("presentation_part"), f"{path}.presentation_part")
        result.append(
            {
                "id": wheel_id,
                "bone": bone,
                "presentation_part": part_id,
                "mesh": _asset_path(wheel.get("mesh"), f"{path}.mesh"),
                "transform": _transform(wheel.get("transform"), f"{path}.transform"),
            }
        )
        ids.append(wheel_id)
        bones.append(bone)
        part_ids.append(part_id)
    if set(ids) != set(WHEEL_IDS) or len(set(ids)) != 4:
        _fail("physics.wheels", f"wheel ids must be exactly {list(WHEEL_IDS)}")
    _unique(bones, "physics.wheels.bone")
    _unique(part_ids, "physics.wheels.presentation_part")
    return result


def _validate_articulations(
    presentation: dict[str, Any],
    explicit_part_ids: set[str],
    mode: str,
) -> list[dict[str, Any]]:
    raw_items = presentation.get("articulations", [])
    if not isinstance(raw_items, list):
        _fail("presentation.articulations", "must be an array")
    ids: list[str] = []
    owned: set[str] = set()
    result: list[dict[str, Any]] = []
    for index, raw in enumerate(raw_items):
        path = f"presentation.articulations[{index}]"
        item = _mapping(raw, path)
        articulation_id = _string(item.get("id"), f"{path}.id")
        pivot = _triple(item.get("pivot"), f"{path}.pivot")
        axis = _triple(item.get("axis"), f"{path}.axis", nonzero=True)
        angle = item.get("open_angle_degrees")
        travel = item.get("travel_seconds")
        if isinstance(angle, bool) or not isinstance(angle, (int, float)) or not math.isfinite(float(angle)):
            _fail(f"{path}.open_angle_degrees", "must be finite")
        if isinstance(travel, bool) or not isinstance(travel, (int, float)) or not math.isfinite(float(travel)) or float(travel) <= 0.0:
            _fail(f"{path}.travel_seconds", "must be finite and positive")
        raw_parts = item.get("parts")
        if not isinstance(raw_parts, list) or not raw_parts:
            _fail(f"{path}.parts", "must contain at least one part")
        parts = [_string(part, f"{path}.parts") for part in raw_parts]
        _unique(parts, f"{path}.parts")
        for part in parts:
            if part in owned:
                _fail("presentation.articulations", f"part {part} belongs to more than one articulation")
            if mode == "explicit" and part not in explicit_part_ids:
                _fail(f"{path}.parts", f"unknown explicit part {part}")
            owned.add(part)
        ids.append(articulation_id)
        result.append(
            {
                "id": articulation_id,
                "pivot": pivot,
                "axis": axis,
                "open_angle_degrees": float(angle),
                "travel_seconds": float(travel),
                "parts": parts,
            }
        )
    _unique(ids, "presentation.articulations.id")
    return result


def _validate_steering(presentation: dict[str, Any]) -> dict[str, Any] | None:
    raw = presentation.get("steering")
    if raw is None:
        return None
    steering = _mapping(raw, "presentation.steering")
    return {
        "part": _string(steering.get("part"), "presentation.steering.part"),
        "pivot": _triple(steering.get("pivot"), "presentation.steering.pivot"),
        "axis": _triple(
            steering.get("axis"), "presentation.steering.axis", nonzero=True
        ),
    }


def normalize_manifest(manifest: dict[str, Any]) -> dict[str, Any]:
    source = copy.deepcopy(_mapping(manifest, "manifest"))
    if source.get("schema_version") != SCHEMA_VERSION:
        _fail("schema_version", f"expected {SCHEMA_VERSION}")

    slug = _string(source.get("slug"), "slug").lower()
    if not SLUG_RE.fullmatch(slug):
        _fail("slug", "must match [a-z0-9][a-z0-9_-]*")

    provenance = _mapping(source.get("source"), "source")
    source_mode = _string(provenance.get("mode"), "source.mode")
    if source_mode not in {"existing_assets", "blender", "glb"}:
        _fail("source.mode", "must be existing_assets, blender, or glb")
    normalized_source: dict[str, Any] = {
        "path": _string(provenance.get("path"), "source.path"),
        "provenance": _string(provenance.get("provenance"), "source.provenance"),
        "mode": source_mode,
    }
    if provenance.get("root") is not None:
        normalized_source["root"] = _string(provenance.get("root"), "source.root")

    physics = _mapping(source.get("physics"), "physics")
    wheels = _validate_wheels(physics)

    presentation = _mapping(source.get("presentation"), "presentation")
    mode = _string(presentation.get("mode"), "presentation.mode")
    if mode not in {"explicit", "discover"}:
        _fail("presentation.mode", "must be explicit or discover")
    palette, rules = _validate_materials(presentation)
    parts, explicit_ids = _validate_parts(presentation, palette)
    if mode == "explicit" and not parts:
        _fail("presentation.parts", "explicit presentation needs at least one part")

    driver = _mapping(source.get("driver"), "driver")
    result: dict[str, Any] = {
        "schema_version": SCHEMA_VERSION,
        "slug": slug,
        "vehicle_id": _string(source.get("vehicle_id"), "vehicle_id"),
        "definition_asset": _asset_path(source.get("definition_asset"), "definition_asset"),
        "source": normalized_source,
        "physics": {
            "carrier_mesh": _asset_path(physics.get("carrier_mesh"), "physics.carrier_mesh"),
            "physics_asset": _asset_path(physics.get("physics_asset"), "physics.physics_asset"),
            "wheels": wheels,
        },
        "presentation": {
            "mode": mode,
            "asset_root": _asset_path(presentation.get("asset_root"), "presentation.asset_root"),
            "root_transform": _transform(
                presentation.get("root_transform"), "presentation.root_transform"
            ),
            "parts": parts,
            "material_palette": palette,
            "material_rules": rules,
            "articulations": _validate_articulations(presentation, explicit_ids, mode),
            "steering": _validate_steering(presentation),
            "cockpit_bindings": copy.deepcopy(presentation.get("cockpit_bindings", [])),
        },
        "driver": {
            "mesh": _asset_path(driver.get("mesh"), "driver.mesh"),
            "transform": _transform(driver.get("transform"), "driver.transform"),
            "head_transform": _transform(
                driver.get("head_transform"), "driver.head_transform"
            ),
        },
    }

    if mode == "discover":
        discovery = _mapping(presentation.get("discovery"), "presentation.discovery")
        id_format = _string(
            discovery.get("id_format"), "presentation.discovery.id_format"
        )
        if "{index" not in id_format:
            _fail(
                "presentation.discovery.id_format",
                "must contain an {index} placeholder",
            )
        raw_excluded = discovery.get("exclude_leafs", [])
        if not isinstance(raw_excluded, list):
            _fail("presentation.discovery.exclude_leafs", "must be an array")
        result["presentation"]["discovery"] = {
            "id_format": id_format,
            "exclude_leafs": [
                _string(value, "presentation.discovery.exclude_leafs")
                for value in raw_excluded
            ],
        }

    return result


def validate_manifest(manifest: dict[str, Any]) -> dict[str, Any]:
    return normalize_manifest(manifest)


def manifest_digest(manifest: dict[str, Any]) -> str:
    normalized = normalize_manifest(manifest)
    payload = json.dumps(
        normalized,
        ensure_ascii=False,
        separators=(",", ":"),
        sort_keys=True,
    ).encode("utf-8")
    return hashlib.sha256(payload).hexdigest()


def load_manifest(path: str | Path) -> dict[str, Any]:
    manifest_path = Path(path)
    data = json.loads(manifest_path.read_text(encoding="utf-8"))
    return validate_manifest(data)
