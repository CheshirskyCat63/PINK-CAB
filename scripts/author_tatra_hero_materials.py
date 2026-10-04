import unreal
import os
import struct
import zlib

ROOT = "/Game/Dev/Vehicles/Tatra613Presentation"
MAT_ROOT = ROOT + "/Materials"
SOURCE_ROOT = os.path.join(
    os.path.dirname(os.path.dirname(os.path.abspath(__file__))),
    "SourceAssets", "Derived", "vehicle", "tatra613"
)
LIB = unreal.MaterialEditingLibrary
TOOLS = unreal.AssetToolsHelpers.get_asset_tools()

def png_chunk(kind, data):
    raw = kind + data
    return struct.pack(">I", len(data)) + raw + struct.pack(">I", zlib.crc32(raw) & 0xFFFFFFFF)

def ensure_noise_png():
    os.makedirs(SOURCE_ROOT, exist_ok=True)
    path = os.path.join(SOURCE_ROOT, "T_TatraMicroNoise.png")
    w = h = 128
    rows = []
    for y in range(h):
        row = bytearray([0])
        for x in range(w):
            n = ((x * 73) ^ (y * 151) ^ ((x * y) * 17) ^ 0x613) & 255
            weave = 18 if (x % 7 == 0 or y % 9 == 0) else 0
            v = max(0, min(255, 108 + ((n - 128) // 5) + weave))
            row.extend((v, v, v, 255))
        rows.append(bytes(row))
    raw = b"".join(rows)
    data = b"\x89PNG\r\n\x1a\n"
    data += png_chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 6, 0, 0, 0))
    data += png_chunk(b"IDAT", zlib.compress(raw, 9))
    data += png_chunk(b"IEND", b"")
    with open(path, "wb") as f:
        f.write(data)
    return path

def import_texture():
    path = ensure_noise_png()
    unreal.EditorAssetLibrary.make_directory(MAT_ROOT)
    asset_path = MAT_ROOT + "/T_TatraMicroNoise"
    texture = unreal.load_asset(asset_path)
    if texture:
        return texture
    task = unreal.AssetImportTask()
    task.filename = path
    task.destination_path = MAT_ROOT
    task.destination_name = "T_TatraMicroNoise"
    task.automated = True
    task.replace_existing = False
    task.save = False
    TOOLS.import_asset_tasks([task])
    texture = unreal.load_asset(asset_path)
    if not texture:
        raise RuntimeError("micro-noise import failed")
    texture.set_editor_property("srgb", False)
    unreal.EditorAssetLibrary.save_loaded_asset(texture, only_if_is_dirty=False)
    return texture

def expr(mat, cls, x=0, y=0):
    return LIB.create_material_expression(mat, cls, x, y)

def constant(mat, value, x, y):
    n = expr(mat, unreal.MaterialExpressionConstant, x, y)
    n.set_editor_property("r", float(value))
    return n

def color(mat, rgba, x, y):
    n = expr(mat, unreal.MaterialExpressionConstant3Vector, x, y)
    n.set_editor_property("constant", unreal.LinearColor(*rgba))
    return n

def scalar(mat, name, value, x, y):
    n = expr(mat, unreal.MaterialExpressionScalarParameter, x, y)
    n.set_editor_property("parameter_name", name)
    n.set_editor_property("default_value", float(value))
    return n

def vector(mat, name, rgba, x, y):
    n = expr(mat, unreal.MaterialExpressionVectorParameter, x, y)
    n.set_editor_property("parameter_name", name)
    n.set_editor_property("default_value", unreal.LinearColor(*rgba))
    return n

def connect(node, output, prop):
    if not LIB.connect_material_property(node, output, prop):
        raise RuntimeError("material property connection failed: " + str(prop))

def configure_existing_surface(material, base, metallic, roughness):
    wanted = {
        "BaseColor": unreal.LinearColor(*base),
        "Metallic": float(metallic),
        "Roughness": float(roughness),
    }
    seen = set()
    for node in LIB.get_material_expressions(material):
        cls = node.get_class().get_name()
        if cls not in ("MaterialExpressionVectorParameter", "MaterialExpressionScalarParameter"):
            continue
        name = str(node.get_editor_property("parameter_name"))
        if name not in wanted:
            continue
        if name in seen:
            raise RuntimeError("duplicate material parameter: " + name)
        seen.add(name)
        if cls == "MaterialExpressionVectorParameter" and name == "BaseColor":
            node.set_editor_property("default_value", wanted[name])
        elif cls == "MaterialExpressionScalarParameter" and name in ("Metallic", "Roughness"):
            node.set_editor_property("default_value", wanted[name])
        else:
            raise RuntimeError("unexpected parameter type: " + name)
    if seen != set(wanted):
        raise RuntimeError("existing surface schema mismatch: " + str(sorted(set(wanted) - seen)))
    errors = LIB.recompile_material(material)
    if errors is None or len(errors):
        raise RuntimeError(material.get_name() + " compile errors: " + str(errors))
    if not unreal.EditorAssetLibrary.save_loaded_asset(material, only_if_is_dirty=False):
        raise RuntimeError(material.get_name() + " save failed")
    return material

def make_surface(name, base, metallic, roughness, noise=None, noise_amount=0.0):
    asset_path = MAT_ROOT + "/" + name
    existing = unreal.load_asset(asset_path)
    if existing:
        return configure_existing_surface(existing, base, metallic, roughness)
    mat = TOOLS.create_asset(name, MAT_ROOT, unreal.Material, unreal.MaterialFactoryNew())
    if not mat:
        raise RuntimeError("material create failed: " + name)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    base_node = vector(mat, "BaseColor", base, -520, -160)
    metal_node = scalar(mat, "Metallic", metallic, -520, -40)
    rough_node = scalar(mat, "Roughness", roughness, -520, 80)
    connect(base_node, "", unreal.MaterialProperty.MP_BASE_COLOR)
    connect(metal_node, "", unreal.MaterialProperty.MP_METALLIC)
    if noise and noise_amount > 0.0:
        uv = expr(mat, unreal.MaterialExpressionTextureCoordinate, -720, 180)
        uv.set_editor_property("u_tiling", 18.0)
        uv.set_editor_property("v_tiling", 18.0)
        tex = expr(mat, unreal.MaterialExpressionTextureSample, -500, 190)
        tex.set_editor_property("texture", noise)
        tex.set_editor_property("sampler_type", unreal.MaterialSamplerType.SAMPLERTYPE_LINEAR_COLOR)
        LIB.connect_material_expressions(uv, "", tex, "UVs")
        mask = expr(mat, unreal.MaterialExpressionComponentMask, -300, 190)
        mask.set_editor_property("r", True)
        LIB.connect_material_expressions(tex, "", mask, "Input")
        bias = constant(mat, noise_amount, -300, 280)
        mul = expr(mat, unreal.MaterialExpressionMultiply, -100, 170)
        LIB.connect_material_expressions(mask, "", mul, "A")
        LIB.connect_material_expressions(bias, "", mul, "B")
        add = expr(mat, unreal.MaterialExpressionAdd, 90, 120)
        LIB.connect_material_expressions(rough_node, "", add, "A")
        LIB.connect_material_expressions(mul, "", add, "B")
        connect(add, "", unreal.MaterialProperty.MP_ROUGHNESS)
    else:
        connect(rough_node, "", unreal.MaterialProperty.MP_ROUGHNESS)
    errors = LIB.recompile_material(mat)
    if errors is None or len(errors):
        raise RuntimeError(name + " compile errors: " + str(errors))
    unreal.EditorAssetLibrary.save_loaded_asset(mat, only_if_is_dirty=False)
    return mat

def make_glass():
    name = "M_PC_Tatra_Glass"
    existing = unreal.load_asset(MAT_ROOT + "/" + name)
    if existing:
        return existing
    mat = TOOLS.create_asset(name, MAT_ROOT, unreal.Material, unreal.MaterialFactoryNew())
    mat.set_editor_property("blend_mode", unreal.BlendMode.BLEND_TRANSLUCENT)
    mat.set_editor_property("shading_model", unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    mat.set_editor_property("two_sided", True)
    base = vector(mat, "GlassTint", (0.020, 0.032, 0.042, 1.0), -420, -80)
    rough = scalar(mat, "Roughness", 0.12, -420, 30)
    opacity = scalar(mat, "Opacity", 0.20, -420, 140)
    connect(base, "", unreal.MaterialProperty.MP_BASE_COLOR)
    connect(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    connect(opacity, "", unreal.MaterialProperty.MP_OPACITY)
    errors = LIB.recompile_material(mat)
    if errors is None or len(errors):
        raise RuntimeError("glass compile errors: " + str(errors))
    unreal.EditorAssetLibrary.save_loaded_asset(mat, only_if_is_dirty=False)
    return mat

def make_light(name, base_rgba, emissive_scale):
    existing = unreal.load_asset(MAT_ROOT + "/" + name)
    if existing:
        return existing
    mat = TOOLS.create_asset(name, MAT_ROOT, unreal.Material, unreal.MaterialFactoryNew())
    base = vector(mat, "LensColor", base_rgba, -450, -50)
    emissive = vector(mat, "EmissiveColor",
                      tuple(c * emissive_scale for c in base_rgba[:3]) + (1.0,),
                      -450, 100)
    rough = scalar(mat, "Roughness", 0.18, -450, 210)
    connect(base, "", unreal.MaterialProperty.MP_BASE_COLOR)
    connect(emissive, "", unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    connect(rough, "", unreal.MaterialProperty.MP_ROUGHNESS)
    errors = LIB.recompile_material(mat)
    if errors is None or len(errors):
        raise RuntimeError(name + " compile errors: " + str(errors))
    unreal.EditorAssetLibrary.save_loaded_asset(mat, only_if_is_dirty=False)
    return mat

def main():
    noise = import_texture()
    make_surface("M_PC_Tatra_BodyPaint", (0.30, 0.020, 0.060, 1.0), 0.08, 0.34, noise, 0.10)
    make_surface("M_PC_Tatra_Chrome", (0.58, 0.62, 0.66, 1.0), 1.0, 0.14)
    make_surface("M_PC_Tatra_Mirror", (0.72, 0.76, 0.80, 1.0), 1.0, 0.035)
    make_surface("M_PC_Tatra_RubberPlastic", (0.010, 0.012, 0.014, 1.0), 0.0, 0.72, noise, 0.08)
    make_surface("M_PC_Tatra_InteriorVinyl", (0.030, 0.023, 0.020, 1.0), 0.0, 0.62, noise, 0.12)
    make_surface("M_PC_Tatra_Fabric", (0.075, 0.052, 0.046, 1.0), 0.0, 0.82, noise, 0.14)
    make_glass()
    make_light("M_PC_Tatra_LightWhite", (0.72, 0.80, 0.95, 1.0), 2.3)
    make_light("M_PC_Tatra_LightRed", (0.56, 0.012, 0.006, 1.0), 2.0)
    make_light("M_PC_Tatra_LightAmber", (0.75, 0.15, 0.010, 1.0), 2.0)
    unreal.EditorAssetLibrary.save_directory(ROOT, only_if_is_dirty=False, recursive=True)
    unreal.log("CD855_TATRA_HERO_MATERIALS=PASS assets=11")
if __name__ == "__main__":
    main()
