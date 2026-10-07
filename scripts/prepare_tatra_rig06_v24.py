import bpy, hashlib, json, os, shutil, struct
from mathutils import Vector
from pathlib import Path

SRC = r"E:\CHESHIRE_DIVISION\SourceAssets\PINK-CAB\Tatra613\working\TATRA613_CHAOS_RIG_23_MATERIAL_UV.blend"
DST = r"E:\CHESHIRE_DIVISION\SourceAssets\PINK-CAB\Tatra613\working\TATRA613_CHAOS_RIG_24_CABIN_BINDINGS.blend"
SRC_PKG = r"E:\CHESHIRE_DIVISION\SourceAssets\PINK-CAB\Tatra613\working\export\TATRA613_RIG23_UE"
PKG = r"E:\CHESHIRE_DIVISION\SourceAssets\PINK-CAB\Tatra613\working\export\TATRA613_RIG24_UE"
FBX = os.path.join(PKG, "TATRA613_CHAOS_RIG_06_TEXTURED_OPENABLES.fbx")
REPORT = os.path.join(PKG, "RIG24_UE_EXPORT_REPORT.json")

BINDINGS = {
    "Cabin_SpeedometerNeedle": ("t613_needle_speedo", "t613_white_material.006"),
    "Cabin_TachometerNeedle": ("t613_needle_tacho", "t613_white_material.007"),
    "Cabin_FuelNeedle": ("t613_needle_fuel", "t613_white_material.003"),
    "Cabin_TemperatureNeedle": ("t613_needle_temp", "t613_white_material.002"),
}

def geometry_digest():
    h=hashlib.sha256()
    for o in sorted((x for x in bpy.data.objects if x.type=="MESH"), key=lambda x:x.name):
        h.update(o.name.encode())
        for row in o.matrix_world:
            for v in row: h.update(struct.pack("<d",float(v)))
        for v in o.data.vertices:
            for x in v.co: h.update(struct.pack("<d",float(x)))
        for p in o.data.polygons:
            h.update(struct.pack("<I",len(p.vertices)))
            for vi in p.vertices: h.update(struct.pack("<I",int(vi)))
    return h.hexdigest()

def bone_snapshot(exclude=()):
    out={}
    arm=bpy.data.objects["TATRA613_CHAOS_RIG"]
    for b in arm.data.bones:
        if b.name in exclude: continue
        out[b.name]={
            "head":[float(x) for x in b.head_local],
            "tail":[float(x) for x in b.tail_local],
            "roll":float(b.matrix_local.to_euler().y),
            "parent":b.parent.name if b.parent else None,
        }
    return out

bpy.ops.wm.open_mainfile(filepath=SRC)
arm=bpy.data.objects["TATRA613_CHAOS_RIG"]
geo_before=geometry_digest()
existing_before=bone_snapshot(exclude=BINDINGS.keys())

# Author only semantic needle bones from existing source pivot empties.
bpy.ops.object.select_all(action="DESELECT")
arm.hide_set(False)
arm.select_set(True)
bpy.context.view_layer.objects.active=arm
bpy.ops.object.mode_set(mode="EDIT")
root=arm.data.edit_bones.get("root")
if not root: raise RuntimeError("Missing root bone")
arm_inv=arm.matrix_world.inverted()
for bone_name,(pivot_name,mesh_name) in BINDINGS.items():
    if arm.data.edit_bones.get(bone_name):
        arm.data.edit_bones.remove(arm.data.edit_bones[bone_name])
    pivot=bpy.data.objects.get(pivot_name)
    if not pivot: raise RuntimeError("Missing source pivot "+pivot_name)
    M=arm_inv @ pivot.matrix_world
    head=M.translation
    y=(M.to_3x3() @ Vector((0,1,0))).normalized()
    z=(M.to_3x3() @ Vector((0,0,1))).normalized()
    eb=arm.data.edit_bones.new(bone_name)
    eb.head=head
    eb.tail=head+y*0.05
    eb.parent=root
    eb.use_connect=False
    eb.align_roll(z)
bpy.ops.object.mode_set(mode="OBJECT")

weight_report={}
for bone_name,(pivot_name,mesh_name) in BINDINGS.items():
    o=bpy.data.objects.get(mesh_name)
    if not o or o.type!="MESH" or len(o.data.vertices)==0:
        raise RuntimeError("Missing needle mesh "+mesh_name)
    if not any(m.type=="ARMATURE" and m.object==arm for m in o.modifiers):
        raise RuntimeError("Needle not owned by Tatra armature: "+mesh_name)
    before_groups=[g.name for g in o.vertex_groups]
    for g in list(o.vertex_groups):
        o.vertex_groups.remove(g)
    vg=o.vertex_groups.new(name=bone_name)
    vg.add(range(len(o.data.vertices)),1.0,"REPLACE")
    weight_report[bone_name]={
        "pivot":pivot_name,"mesh":mesh_name,"vertices":len(o.data.vertices),
        "polygons":len(o.data.polygons),"groups_before":before_groups,
        "groups_after":[g.name for g in o.vertex_groups],
    }

geo_after=geometry_digest()
if geo_before!=geo_after:
    raise RuntimeError("Geometry digest changed while adding needle bindings")
existing_after=bone_snapshot(exclude=BINDINGS.keys())
if existing_before!=existing_after:
    raise RuntimeError("Existing bone transforms changed while adding needle bindings")

arm["RIG24"]="V23 geometry/UV preserved; 4 instrument needles bound to existing source pivot empties"
arm["RIG24_NeedleBones"]=",".join(BINDINGS.keys())
arm["RIG24_NeedlePolicy"]="Existing presentation state drives source-pivot bones; no generated cockpit needle geometry"
bpy.ops.wm.save_as_mainfile(filepath=DST)

os.makedirs(PKG,exist_ok=True)
shutil.copytree(os.path.join(SRC_PKG,"Textures"),os.path.join(PKG,"Textures"),dirs_exist_ok=True)
manifest_path=os.path.join(PKG,"Textures","UE_MATERIAL_MANIFEST.json")
if os.path.isfile(manifest_path):
    with open(manifest_path,"r",encoding="utf-8-sig") as f: manifest=json.load(f)
    manifest["asset"]="TATRA613_CHAOS_RIG_24_CABIN_BINDINGS"
    manifest["source_blend"]=DST
    manifest["instrument_binding_policy"]="4 source needle meshes bound 100% to source-pivot semantic bones; geometry unchanged"
    with open(manifest_path,"w",encoding="utf-8") as f: json.dump(manifest,f,indent=2,ensure_ascii=False)

def enable_layer_tree(lc):
    lc.exclude=False; lc.hide_viewport=False
    for child in lc.children: enable_layer_tree(child)
enable_layer_tree(bpy.context.view_layer.layer_collection)
bpy.ops.object.select_all(action="DESELECT")
selected=[]
for o in list(bpy.context.view_layer.objects):
    if o.type not in {"ARMATURE","MESH"}: continue
    if o.get("debug_guide") or o.get("export_to_ue") is False: continue
    o.hide_set(False);o.select_set(True);selected.append(o.name)
arm.data.pose_position="REST"
if arm.animation_data: arm.animation_data.action=None
bpy.context.scene.frame_set(1);bpy.context.view_layer.update()
bpy.context.view_layer.objects.active=arm
bpy.ops.export_scene.fbx(
    filepath=FBX,use_selection=True,object_types={"ARMATURE","MESH"},
    apply_unit_scale=True,apply_scale_options="FBX_SCALE_UNITS",
    use_mesh_modifiers=True,mesh_smooth_type="FACE",use_tspace=False,
    add_leaf_bones=False,bake_anim=False,path_mode="RELATIVE",embed_textures=False,
    axis_forward="-Y",axis_up="Z")

bones=[b.name for b in arm.data.bones]
required={"root","Phys_Wheel_FL","Phys_Wheel_FR","Phys_Wheel_BL","Phys_Wheel_BR",
          "Steering_Wheel","Cabin_GearLever","Cabin_Handbrake",
          "Cabin_ClutchPedal","Cabin_BrakePedal","Cabin_ThrottlePedal",
          "Door_FL","Door_FR","Door_RL","Door_RR","Trunk_Front","Hood_Rear",
          "Window_FL","Window_FR","Window_RL","Window_RR",*BINDINGS.keys()}
report={
    "source":SRC,"blend":DST,"fbx":FBX,"blender":bpy.app.version_string,
    "geometry_digest_before":geo_before,"geometry_digest_after":geo_after,
    "geometry_unchanged":geo_before==geo_after,
    "existing_bones_unchanged":existing_before==existing_after,
    "instrument_bindings":weight_report,
    "object_count":len(selected),"bone_count":len(bones),
    "required_missing":sorted(required-set(bones)),
    "fbx_bytes":os.path.getsize(FBX),
    "fbx_sha256":hashlib.sha256(open(FBX,"rb").read()).hexdigest(),
}
with open(REPORT,"w",encoding="utf-8") as f: json.dump(report,f,indent=2,ensure_ascii=False)
print("RIG24_UE_EXPORT_OK",json.dumps({
 "geometry_unchanged":report["geometry_unchanged"],
 "existing_bones_unchanged":report["existing_bones_unchanged"],
 "object_count":report["object_count"],"bone_count":report["bone_count"],
 "required_missing":report["required_missing"],"fbx_sha256":report["fbx_sha256"]}))
