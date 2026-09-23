import bpy, json, os
from mathutils import Vector
project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
src = os.environ.get("PINKCAB_TATRA_SOURCE_GLTF")
if not src:
    raise RuntimeError("PINKCAB_TATRA_SOURCE_GLTF is required")
out = os.path.join(project_root, "Saved", "TatraV12_BlenderReport.json")
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=src)
rows=[]
for obj in bpy.context.scene.objects:
    if obj.type != 'MESH':
        continue
    corners=[obj.matrix_world @ Vector(c) for c in obj.bound_box]
    xs=[v.x for v in corners]; ys=[v.y for v in corners]; zs=[v.z for v in corners]
    rows.append({'name':obj.name,'verts':len(obj.data.vertices),'dims':[max(xs)-min(xs),max(ys)-min(ys),max(zs)-min(zs)],'center':[(max(xs)+min(xs))/2,(max(ys)+min(ys))/2,(max(zs)+min(zs))/2],'materials':[m.name if m else None for m in obj.data.materials]})
rows.sort(key=lambda r:r['verts'], reverse=True)
with open(out,'w',encoding='utf-8') as f:
    json.dump({'count':len(rows),'rows':rows},f,ensure_ascii=False,indent=2)
print('REPORT',out,'MESHES',len(rows))
