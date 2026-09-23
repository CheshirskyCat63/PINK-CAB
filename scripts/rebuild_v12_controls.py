import bpy, os
from mathutils import Vector
src=os.environ.get("PINKCAB_TATRA_SOURCE_GLTF")
outdir=os.environ.get("PINKCAB_TATRA_V12_CLEAN_DIR")
if not src or not outdir:
    raise RuntimeError("PINKCAB_TATRA_SOURCE_GLTF and PINKCAB_TATRA_V12_CLEAN_DIR are required")
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=src)
def descendants(root):
 out=[]
 def rec(o):
  if o.type=='MESH': out.append(o)
  for c in o.children: rec(c)
 rec(root); return out
def export_centered(names, filename):
 bpy.ops.object.select_all(action='DESELECT')
 chosen=[]
 for n in names: chosen += descendants(bpy.data.objects[n])
 chosen=list(dict.fromkeys(chosen))
 for o in chosen: o.select_set(True)
 bpy.context.view_layer.objects.active=chosen[0]
 bpy.ops.object.join(); obj=bpy.context.view_layer.objects.active
 bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
 pts=[Vector(c) for c in obj.bound_box]
 center=sum(pts,Vector())/8.0
 obj.data.transform(__import__('mathutils').Matrix.Translation(-center))
 obj.location=(0,0,0); obj.rotation_euler=(0,0,0); obj.scale=(1,1,1)
 obj.name=os.path.splitext(filename)[0]
 bpy.ops.export_scene.gltf(filepath=os.path.join(outdir,filename),export_format='GLB',use_selection=True,export_apply=True)
 print('CENTERED',filename,'verts',len(obj.data.vertices),'center',center)
export_centered({'t613_steer'},'Tatra613_V12_Steering.glb')
export_centered({'wheel.002','wheel.001','wheel'},'Tatra613_V12_Wheel.glb')
