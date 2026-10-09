import bpy
import hashlib
import json
import math
import os
import shutil
import struct

SRC = r"E:\CHESHIRE_DIVISION\SourceAssets\PINK-CAB\Tatra613\working\TATRA613_CHAOS_RIG_22_FINAL.blend"
DST = r"E:\CHESHIRE_DIVISION\SourceAssets\PINK-CAB\Tatra613\working\TATRA613_CHAOS_RIG_23_MATERIAL_UV.blend"
SRC_PKG = r"E:\CHESHIRE_DIVISION\SourceAssets\PINK-CAB\Tatra613\working\export\TATRA613_RIG22_UE"
PKG = r"E:\CHESHIRE_DIVISION\SourceAssets\PINK-CAB\Tatra613\working\export\TATRA613_RIG23_UE"
FBX = os.path.join(PKG, "TATRA613_CHAOS_RIG_06_TEXTURED_OPENABLES.fbx")
REPORT = os.path.join(PKG, "RIG23_UE_EXPORT_REPORT.json")
DONOR_NAMES = (
    "DONOR_Pedal_Clutch", "DONOR_Pedal_Brake", "DONOR_Pedal_Throttle",
    "DONOR_Mount_Clutch", "DONOR_Mount_Brake", "DONOR_Mount_Throttle",
    "DONOR_Bearing_Clutch", "DONOR_Bearing_Brake", "DONOR_Bearing_Throttle",
    "DONOR_Spacer_Clutch", "DONOR_Spacer_Brake", "DONOR_Spacer_Throttle",
)

def digest_geometry():
    h = hashlib.sha256()
    meshes = sorted((o for o in bpy.data.objects if o.type == "MESH"), key=lambda o:o.name)
    for o in meshes:
        h.update(o.name.encode("utf-8"))
        for row in o.matrix_world:
            for v in row:
                h.update(struct.pack("<d", float(v)))
        for v in o.data.vertices:
            for x in v.co:
                h.update(struct.pack("<d", float(x)))
        for p in o.data.polygons:
            h.update(struct.pack("<I", len(p.vertices)))
            for vi in p.vertices:
                h.update(struct.pack("<I", int(vi)))
    arm = bpy.data.objects["TATRA613_CHAOS_RIG"]
    for b in arm.data.bones:
        h.update(b.name.encode("utf-8"))
        for x in (*b.head_local, *b.tail_local):
            h.update(struct.pack("<d", float(x)))
    return h.hexdigest()

bpy.ops.wm.open_mainfile(filepath=SRC)
arm = bpy.data.objects["TATRA613_CHAOS_RIG"]
before = digest_geometry()

uv_report = {}
for name in DONOR_NAMES:
    o = bpy.data.objects.get(name)
    if not o or o.type != "MESH" or len(o.data.polygons) == 0:
        raise RuntimeError("Missing/non-mesh donor object: " + name)

    old_layers = len(o.data.uv_layers)
    if old_layers == 0:
        o.data.uv_layers.new(name="UVMap")
        bpy.ops.object.select_all(action="DESELECT")
        o.hide_set(False)
        o.select_set(True)
        bpy.context.view_layer.objects.active = o
        bpy.ops.object.mode_set(mode="EDIT")
        bpy.ops.mesh.select_all(action="SELECT")
        bpy.ops.uv.smart_project(
            angle_limit=math.radians(66.0),
            island_margin=0.03,
            correct_aspect=True,
            scale_to_bounds=False,
        )
        bpy.ops.object.mode_set(mode="OBJECT")
        o.select_set(False)

    uv = o.data.uv_layers.active
    if not uv or len(uv.data) != len(o.data.loops):
        raise RuntimeError("Invalid UV0 after unwrap: " + name)
    uv_report[name] = {
        "polygons": len(o.data.polygons),
        "loops": len(o.data.loops),
        "uv_layers_before": old_layers,
        "uv_layers_after": len(o.data.uv_layers),
        "uv_data": len(uv.data),
    }

after = digest_geometry()
if before != after:
    raise RuntimeError("Geometry/bone digest changed while adding UVs")

arm["material_uv_v23"] = "UV0 smart-project only on 12 SimPedals donor meshes; geometry/bones unchanged"
bpy.ops.wm.save_as_mainfile(filepath=DST)

os.makedirs(PKG, exist_ok=True)
src_tex = os.path.join(SRC_PKG, "Textures")
dst_tex = os.path.join(PKG, "Textures")
shutil.copytree(src_tex, dst_tex, dirs_exist_ok=True)
manifest_path = os.path.join(dst_tex, "UE_MATERIAL_MANIFEST.json")
if os.path.isfile(manifest_path):
    with open(manifest_path, "r", encoding="utf-8-sig") as f:
        manifest = json.load(f)
    manifest["asset"] = "TATRA613_CHAOS_RIG_23_MATERIAL_UV"
    manifest["source_blend"] = DST
    manifest["normal_policy"] = "FBX authored normals; UE imports normals and generates MikkTSpace tangents"
    manifest["uv_policy"] = "SimPedals donor geometry has authored UV0; no geometry/bone transform change"
    with open(manifest_path, "w", encoding="utf-8") as f:
        json.dump(manifest, f, indent=2, ensure_ascii=False)

def enable_layer_tree(layer_coll):
    layer_coll.exclude = False
    layer_coll.hide_viewport = False
    for child in layer_coll.children:
        enable_layer_tree(child)

enable_layer_tree(bpy.context.view_layer.layer_collection)
bpy.ops.object.select_all(action="DESELECT")
selected = []
for o in list(bpy.context.view_layer.objects):
    if o.type not in {"ARMATURE", "MESH"}:
        continue
    if o.get("debug_guide") or o.get("export_to_ue") is False:
        continue
    o.hide_set(False)
    o.select_set(True)
    selected.append(o.name)

arm.data.pose_position = "REST"
if arm.animation_data:
    arm.animation_data.action = None
bpy.context.scene.frame_set(1)
bpy.context.view_layer.update()
bpy.context.view_layer.objects.active = arm

bpy.ops.export_scene.fbx(
    filepath=FBX,
    use_selection=True,
    object_types={"ARMATURE", "MESH"},
    apply_unit_scale=True,
    apply_scale_options="FBX_SCALE_UNITS",
    use_mesh_modifiers=True,
    mesh_smooth_type="FACE",
    use_tspace=False,
    add_leaf_bones=False,
    bake_anim=False,
    path_mode="RELATIVE",
    embed_textures=False,
    axis_forward="-Y",
    axis_up="Z",
)

required={"root","Phys_Wheel_FL","Phys_Wheel_FR","Phys_Wheel_BL","Phys_Wheel_BR",
          "Steering_Wheel","Cabin_GearLever","Cabin_Handbrake",
          "Door_FL","Door_FR","Door_RL","Door_RR","Trunk_Front","Hood_Rear"}
bones=[b.name for b in arm.data.bones]
materials=sorted({m.name for o in bpy.context.view_layer.objects if o.type=="MESH" and o.name in selected for m in o.data.materials if m})
report={
    "source": SRC,
    "blend": DST,
    "fbx": FBX,
    "blender": bpy.app.version_string,
    "geometry_bone_digest_before": before,
    "geometry_bone_digest_after": after,
    "geometry_bones_unchanged": before == after,
    "uv_report": uv_report,
    "selected_objects": selected,
    "object_count": len(selected),
    "bone_count": len(bones),
    "required_missing": sorted(required-set(bones)),
    "materials": materials,
    "use_tspace": False,
    "normal_policy": "authored FBX normals; MikkTSpace tangents generated in UE",
    "fbx_bytes": os.path.getsize(FBX),
    "fbx_sha256": hashlib.sha256(open(FBX, "rb").read()).hexdigest(),
}
with open(REPORT, "w", encoding="utf-8") as f:
    json.dump(report, f, indent=2, ensure_ascii=False)
print("RIG23_UE_EXPORT_OK", json.dumps({k:report[k] for k in (
    "blender","geometry_bones_unchanged","object_count","bone_count",
    "required_missing","use_tspace","fbx_bytes","fbx_sha256"
)}, ensure_ascii=False))
