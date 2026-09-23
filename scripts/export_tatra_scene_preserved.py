import bpy, os, json, math
from mathutils import Vector

PROJECT_ROOT=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT=os.path.join(PROJECT_ROOT,"Saved","Tatra613ScenePreserved")
os.makedirs(OUT,exist_ok=True)

NATIVE_WHEEL_MESHES={
 "t613_Black_material.005",
 "t613_Black_material.006",
 "t613_chrome_material",
}
all_meshes=[o for o in bpy.context.scene.objects if o.type=='MESH']
meshes=[o for o in all_meshes if o.name not in NATIVE_WHEEL_MESHES]
report={
 "source":bpy.data.filepath,
 "source_mesh_count":len(all_meshes),
 "runtime_mesh_count":len(meshes),
 "excluded_native_wheel_meshes":sorted(NATIVE_WHEEL_MESHES),
 "verts":sum(len(o.data.vertices) for o in meshes),
 "polys":sum(len(o.data.polygons) for o in meshes),
 "objects":[]
}
for o in meshes:
 report["objects"].append({
  "name":o.name,
  "location":[float(x) for x in o.matrix_world.translation],
  "rotation":[float(x) for x in o.matrix_world.to_euler()],
  "scale":[float(x) for x in o.matrix_world.to_scale()],
  "verts":len(o.data.vertices),
  "polys":len(o.data.polygons),
  "materials":[m.name if m else None for m in o.data.materials],
 })
glass=bpy.data.materials.get("PC_T613_GlassSlot")
if not glass:
 glass=bpy.data.materials.new("PC_T613_GlassSlot")
 glass.use_nodes=True
 glass.diffuse_color=(0.05,0.07,0.09,0.16)

glass_objects=[]
for o in meshes:
 if o.name.startswith("t613_glass_material"):
  o.data=o.data.copy()
  o.data.materials.clear()
  o.data.materials.append(glass)
  glass_objects.append(o.name)

bpy.ops.object.select_all(action='DESELECT')
for o in meshes:
 o.select_set(True)
bpy.context.view_layer.objects.active=meshes[0]
bpy.ops.export_scene.gltf(
 filepath=os.path.join(OUT,"Tatra613_ScenePreserved.glb"),
 export_format='GLB',
 use_selection=True,
 export_apply=False,
 export_yup=True)
report["glass_objects"]=glass_objects
with open(os.path.join(OUT,"source_scene_report.json"),"w",encoding="utf-8") as f:
 json.dump(report,f,indent=2)
print("TATRA_SCENE_PRESERVED_EXPORT_OK",len(meshes),report["verts"],report["polys"],"excluded",sorted(NATIVE_WHEEL_MESHES))
