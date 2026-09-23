import json
import os
import sys
import unreal

STALE_ROOTS = [
    "/Game/Dev/Vehicles/Tatra613Donor",
    "/Game/Dev/Vehicles/Tatra613ArchiveV12",
    "/Game/Dev/Vehicles/Tatra613SceneProbe",
    "/Game/Dev/Vehicles/Tatra613DesktopClean",
    "/Game/Dev/Vehicles/Tatra613DesktopFaithful",
    "/Game/Dev/Vehicles/Tatra613ArchiveV12Clean/Tatra613_V12_Body",
    "/Game/Dev/Vehicles/Tatra613ArchiveV12Clean/Tatra613_V12_Steering",
]

ACTIVE_ROOTS = [
    "/Game/Dev/Vehicles/Tatra613DesktopScene",
    "/Game/Dev/Vehicles/Tatra613ArchiveV12Clean/Tatra613_V12_Wheel",
]

registry = unreal.AssetRegistryHelpers.get_asset_registry()
registry.wait_for_completion()

rows = []
external_total = 0
asset_total = 0

def under_any(path, roots):
    return any(path == root or path.startswith(root + "/") for root in roots)

for root in STALE_ROOTS:
    assets = registry.get_assets_by_path(root, recursive=True)
    root_external = set()
    for asset in assets:
        asset_total += 1
        package_name = str(asset.package_name)
        try:
            refs = unreal.EditorAssetLibrary.find_package_referencers_for_asset(
                package_name, load_assets_to_confirm=True
            )
        except Exception as exc:
            unreal.log_error(f"TATRA_REF_SCAN_ERROR asset={package_name} error={exc}")
            raise
        external = sorted({
            str(ref) for ref in refs
            if not under_any(str(ref), STALE_ROOTS)
        })
        if external:
            root_external.update(external)
            rows.append({
                "root": root,
                "asset": package_name,
                "external_referencers": external,
            })
    external_total += len(root_external)

project_dir = unreal.Paths.project_dir()
out_dir = os.path.join(project_dir, "Saved", "GitHubGate")
os.makedirs(out_dir, exist_ok=True)
out_path = os.path.join(out_dir, "TATRA_STALE_REFERENCE_SCAN.json")
payload = {
    "stale_roots": STALE_ROOTS,
    "active_roots": ACTIVE_ROOTS,
    "asset_count": asset_total,
    "assets_with_external_referencers": len(rows),
    "external_referencer_root_total": external_total,
    "rows": rows,
}
with open(out_path, "w", encoding="utf-8") as handle:
    json.dump(payload, handle, indent=2, ensure_ascii=False)

unreal.log(
    f"TATRA_STALE_REFERENCE_SCAN_DONE assets={asset_total} "
    f"assets_with_external_refs={len(rows)} report={out_path}"
)
