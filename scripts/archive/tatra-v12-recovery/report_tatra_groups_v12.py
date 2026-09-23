import bpy, json, os
from mathutils import Vector
project_root=os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
src=os.environ.get("PINKCAB_TATRA_SOURCE_GLTF")
if not src:
    raise RuntimeError("PINKCAB_TATRA_SOURCE_GLTF is required")
out=os.path.join(project_root,"Saved","TatraV12_Groups.json")
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=src)
def meshes_under(o):
    out=[]
    def rec(x):
        if x.type=='MESH': out.append(x)
        for c in x.children: rec(c)
    rec(o); return out
rows=[]
for o in bpy.context.scene.objects:
    if o.parent and o.parent.name=='Collada visual scene group' and (o.name.startswith('t613') or o.name.startswith('wheel') or o.name.startswith('bottom')):
        ms=meshes_under(o); pts=[]; verts=0
        for m in ms:
            verts+=len(m.data.vertices)
            pts += [m.matrix_world @ Vector(c) for c in m.bound_box]
        if pts:
            xs=[p.x for p in pts]; ys=[p.y for p in pts]; zs=[p.z for p in pts]
            rows.append({'name':o.name,'verts':verts,'dims':[max(xs)-min(xs),max(ys)-min(ys),max(zs)-min(zs)],'center':[(max(xs)+min(xs))/2,(max(ys)+min(ys))/2,(max(zs)+min(zs))/2]})
with open(out,'w',encoding='utf-8') as f: json.dump(rows,f,indent=2)
print('WROTE',len(rows))
