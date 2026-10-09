import unreal, os, json

dest="/Game/Dev/Vehicles/Tatra613Rig06"
source_root=os.environ.get(
    "PINKCAB_TATRA_RIG06_PACKAGE",
    r"E:\CHESHIRE_DIVISION\SourceAssets\PINK-CAB\Tatra613\working\export\TATRA613_RIG24_UE")
tex_root=os.path.join(source_root,"Textures")
asset_tools=unreal.AssetToolsHelpers.get_asset_tools()
mel=unreal.MaterialEditingLibrary

def load(path):
    a=unreal.load_asset(path)
    if not a:
        raise RuntimeError("Missing asset: "+path)
    return a

# Import any texture missing from the FBX import. The FBX can reference an
# alternate paint source image, while the material package contains the
# canonical BaseColor/Normal/ORM set we want to bind in Unreal.
texture_files=sorted(
    os.path.join(tex_root,f) for f in os.listdir(tex_root)
    if f.lower().endswith(".png"))
orm_files=[f for f in texture_files if f.endswith("_ORM.png")]
tasks=[]
for filename in texture_files:
    name=os.path.splitext(os.path.basename(filename))[0]
    asset_path=dest+"/"+name
    if unreal.EditorAssetLibrary.does_asset_exist(asset_path):
        continue
    task=unreal.AssetImportTask()
    task.filename=filename
    task.destination_path=dest
    task.destination_name=name
    task.automated=True
    task.replace_existing=True
    task.replace_existing_settings=True
    task.save=True
    tasks.append(task)
if tasks:
    asset_tools.import_asset_tasks(tasks)

# Map material assets to their texture set.
mapping={
    "M_PC_BrakeMetal":"Brake",
    "M_PC_Pedal_OEM_BlackSteel":"Steel",
    "M_PC_CarPaint_PinkCab":"PaintPink",
    "M_PC_Chrome_Aged":"Chrome",
    "M_PC_Glass_Clear":"Glass",
    "M_PC_Lens_Amber":"LensAmber",
    "M_PC_Lens_Clear":"LensClear",
    "M_PC_Lens_Red":"LensRed",
    "M_PC_Mirror":"Chrome",
    "M_PC_Plastic_Black":"Plastic",
    "M_PC_Rubber_Tyre":"Rubber",
    "M_PC_Steel_Mechanical":"Steel",
    "M_PC_Underbody_Coating":"Undercoat",
    "M_PC_Velour_Charcoal":"Velour",
    "M_PC_Vinyl_Interior":"Vinyl",
}
indicator_colors={
    "M_PC_Indicator_Red":unreal.LinearColor(0.65,0.01,0.008,1.0),
    "M_PC_Indicator_Amber":unreal.LinearColor(0.80,0.22,0.01,1.0),
    "M_PC_Indicator_Blue":unreal.LinearColor(0.02,0.12,0.72,1.0),
}
transparent={
    "M_PC_Glass_Clear":0.22,
    "M_PC_Lens_Clear":0.48,
    "M_PC_Lens_Red":0.64,
    "M_PC_Lens_Amber":0.64,
}

def set_texture_settings(tex, kind):
    if kind=="base":
        tex.set_editor_property("srgb",True)
    elif kind=="normal":
        tex.set_editor_property("srgb",False)
        try: tex.set_editor_property("compression_settings",unreal.TextureCompressionSettings.TC_NORMALMAP)
        except Exception as e: unreal.log_warning("normal compression: "+str(e))
    else:
        tex.set_editor_property("srgb",False)
        try: tex.set_editor_property("compression_settings",unreal.TextureCompressionSettings.TC_MASKS)
        except Exception as e: unreal.log_warning("mask compression: "+str(e))
    unreal.EditorAssetLibrary.save_loaded_asset(tex,only_if_is_dirty=True)

def clear_material(mat):
    try:
        mel.delete_all_material_expressions(mat)
    except Exception:
        # Fallback to deleting known expression objects.
        for expr in list(mat.get_editor_property("expressions")):
            try: mel.delete_material_expression(mat,expr)
            except Exception: pass

def make_tex(mat,tex,x,y,label):
    e=mel.create_material_expression(mat,unreal.MaterialExpressionTextureSample,x,y)
    e.set_editor_property("texture",tex)
    e.set_editor_property("desc",label)
    return e

def make_constant(mat,value,x,y):
    e=mel.create_material_expression(mat,unreal.MaterialExpressionConstant,x,y)
    e.set_editor_property("r",float(value))
    return e

# Enforce the package texture convention for every authored source texture,
# including currently-unused sets such as Carpet. Asset metadata must not
# depend on whether a material happens to reference that texture today.
for filename in texture_files:
    name=os.path.splitext(os.path.basename(filename))[0]
    tex=load(dest+"/"+name)
    if name.endswith("_BaseColor"):
        kind="base"
    elif name.endswith("_Normal"):
        kind="normal"
    elif name.endswith("_ORM"):
        kind="orm"
    else:
        continue
    set_texture_settings(tex,kind)

configured=[]
for mat_name,set_name in mapping.items():
    mat=load(dest+"/"+mat_name)
    base=load(dest+"/T_PC_"+set_name+"_BaseColor")
    normal=load(dest+"/T_PC_"+set_name+"_Normal")
    orm=load(dest+"/T_PC_"+set_name+"_ORM")
    set_texture_settings(base,"base")
    set_texture_settings(normal,"normal")
    set_texture_settings(orm,"orm")

    clear_material(mat)
    try: mat.set_editor_property("blend_mode",unreal.BlendMode.BLEND_OPAQUE)
    except Exception: pass
    try: mat.set_editor_property("two_sided",False)
    except Exception: pass
    try: mat.set_editor_property("shading_model",unreal.MaterialShadingModel.MSM_DEFAULT_LIT)
    except Exception: pass

    b=make_tex(mat,base,-700,-240,"BaseColor sRGB")
    n=make_tex(mat,normal,-700,80,"Tangent-space Normal")
    o=make_tex(mat,orm,-700,340,"ORM: R=AO G=Roughness B=Metallic")
    try: n.set_editor_property("sampler_type",unreal.MaterialSamplerType.SAMPLERTYPE_NORMAL)
    except Exception: pass
    try: o.set_editor_property("sampler_type",unreal.MaterialSamplerType.SAMPLERTYPE_MASKS)
    except Exception: pass

    mel.connect_material_property(b,"RGB",unreal.MaterialProperty.MP_BASE_COLOR)
    mel.connect_material_property(n,"RGB",unreal.MaterialProperty.MP_NORMAL)
    mel.connect_material_property(o,"R",unreal.MaterialProperty.MP_AMBIENT_OCCLUSION)
    mel.connect_material_property(o,"G",unreal.MaterialProperty.MP_ROUGHNESS)
    mel.connect_material_property(o,"B",unreal.MaterialProperty.MP_METALLIC)

    if mat_name=="M_PC_CarPaint_PinkCab":
        try:
            mat.set_editor_property("shading_model",unreal.MaterialShadingModel.MSM_CLEAR_COAT)
            cc=make_constant(mat,0.82,-260,520)
            cr=make_constant(mat,0.13,-260,600)
            mel.connect_material_property(cc,"",unreal.MaterialProperty.MP_CUSTOM_DATA_0)
            mel.connect_material_property(cr,"",unreal.MaterialProperty.MP_CUSTOM_DATA_1)
        except Exception as e:
            unreal.log_warning("ClearCoat setup fallback to DefaultLit: "+str(e))
    if mat_name in transparent:
        opacity=transparent[mat_name]
        try:
            mat.set_editor_property("blend_mode",unreal.BlendMode.BLEND_TRANSLUCENT)
            mat.set_editor_property("two_sided",True)
        except Exception as e:
            unreal.log_warning("translucent setup: "+str(e))
        op=make_constant(mat,opacity,-260,700)
        mel.connect_material_property(op,"",unreal.MaterialProperty.MP_OPACITY)
        if mat_name=="M_PC_Glass_Clear":
            try:
                ref=make_constant(mat,1.45,-260,780)
                mel.connect_material_property(ref,"",unreal.MaterialProperty.MP_REFRACTION)
            except Exception: pass

    mel.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat,only_if_is_dirty=True)
    configured.append(mat_name)

for mat_name,color in indicator_colors.items():
    mat=load(dest+"/"+mat_name)
    clear_material(mat)
    try: mat.set_editor_property("blend_mode",unreal.BlendMode.BLEND_OPAQUE)
    except Exception: pass
    e=mel.create_material_expression(mat,unreal.MaterialExpressionConstant3Vector,-350,-30)
    e.set_editor_property("constant",color)
    mel.connect_material_property(e,"",unreal.MaterialProperty.MP_BASE_COLOR)
    # low emission so dashboard icons read without becoming neon.
    mel.connect_material_property(e,"",unreal.MaterialProperty.MP_EMISSIVE_COLOR)
    mel.recompile_material(mat)
    unreal.EditorAssetLibrary.save_loaded_asset(mat,only_if_is_dirty=True)
    configured.append(mat_name)

# Remove importer collision duplicates after all canonical materials have been rebuilt.
for asset in unreal.EditorAssetLibrary.list_assets(dest,recursive=False,include_folder=False):
    s=str(asset)
    if "_ncl1_" in s:
        unreal.EditorAssetLibrary.delete_asset(s)

sk=load(dest+"/SK_Tatra613_Rig06")
slot_report=[]
try:
    mats=sk.get_editor_property("materials")
    for i,m in enumerate(mats):
        slot=None
        iface=None
        for prop in ("material_slot_name","imported_material_slot_name"):
            try:
                v=m.get_editor_property(prop)
                if v: slot=str(v)
            except Exception: pass
        try: iface=m.get_editor_property("material_interface")
        except Exception: pass
        slot_report.append({"index":i,"slot":slot,"material":iface.get_name() if iface else None})
except Exception as e:
    unreal.log_warning("material slot audit failed: "+str(e))

bounds=None
try:
    b=sk.get_bounds()
    bounds={
        "origin":[b.origin.x,b.origin.y,b.origin.z],
        "box_extent":[b.box_extent.x,b.box_extent.y,b.box_extent.z],
        "sphere_radius":b.sphere_radius,
    }
except Exception as e:
    unreal.log_warning("bounds audit unavailable: "+str(e))

unreal.EditorAssetLibrary.save_directory(dest,only_if_is_dirty=True,recursive=True)
report={
    "configured_materials":configured,
    "orm_imported":[os.path.basename(x) for x in orm_files],
    "slots":slot_report,
    "bounds":bounds,
    "skeletal_mesh":sk.get_path_name(),
}
out=os.path.join(os.path.dirname(os.path.dirname(os.path.abspath(__file__))),"Saved","Tatra613Rig06Materials_report.json")
with open(out,"w",encoding="utf-8") as f: json.dump(report,f,indent=2)
unreal.log("TATRA_RIG06_MATERIALS_OK "+json.dumps(report))
