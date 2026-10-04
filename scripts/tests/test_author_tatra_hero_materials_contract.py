from pathlib import Path

SRC = Path(__file__).resolve().parents[1] / "author_tatra_hero_materials.py"
text = SRC.read_text(encoding="utf-8")


def require(token: str, message: str) -> None:
    if token not in text:
        raise SystemExit("TATRA_HERO_MATERIAL_CONTRACT=FAIL " + message)


require("def configure_existing_surface(", "existing surfaces must be updated, not silently reused")
require("existing = unreal.load_asset(asset_path)", "material author must read existing assets")
require("configure_existing_surface(existing, base, metallic, roughness)", "existing surface values must be applied")
require('make_surface("M_PC_Tatra_BodyPaint", (0.30, 0.020, 0.060, 1.0), 0.08, 0.34', "body paint must use dielectric-like metallic and stable roughness")
require("recompile_material", "updated materials must compile")
require("save_loaded_asset", "updated materials must be saved")
print("TATRA_HERO_MATERIAL_CONTRACT=PASS")
