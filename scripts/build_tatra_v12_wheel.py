import bpy, os
from mathutils import Vector, Matrix

src=os.environ.get("PINKCAB_TATRA_SOURCE_GLTF")
outdir=os.environ.get("PINKCAB_TATRA_V12_CLEAN_DIR")
if not src or not outdir:
    raise RuntimeError("PINKCAB_TATRA_SOURCE_GLTF and PINKCAB_TATRA_V12_CLEAN_DIR are required")

os.makedirs(outdir,exist_ok=True)
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=src)

def descendants(root):
    out=[]
    def rec(obj):
        if obj.type=='MESH':
            out.append(obj)
        for child in obj.children:
            rec(child)
    rec(root)
    return out

def select_join(names):
    bpy.ops.object.select_all(action='DESELECT')
    chosen=[]
    for name in names:
        obj=bpy.data.objects.get(name)
        if obj:
            chosen += descendants(obj)
    chosen=list(dict.fromkeys(chosen))
    if not chosen:
        raise RuntimeError("Tatra wheel source selection is empty")
    for obj in chosen:
        obj.select_set(True)
    bpy.context.view_layer.objects.active=chosen[0]
    if len(chosen)>1:
        bpy.ops.object.join()
    return bpy.context.view_layer.objects.active

for mat in bpy.data.materials:
    name=mat.name.lower()
    if 't613_black' in name:
        mat.diffuse_color=(0.012,0.014,0.018,1.0)
    elif 't613_chrome' in name:
        mat.diffuse_color=(0.30,0.32,0.35,1.0)

obj=select_join({'wheel.002','wheel.001','wheel'})
bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
center=sum((Vector(c) for c in obj.bound_box),Vector())/8.0
obj.data.transform(Matrix.Translation(-center))
obj.location=(0,0,0)
obj.rotation_euler=(0,0,0)
obj.scale=(1,1,1)
obj.name='Tatra613_V12_Wheel'

out=os.path.join(outdir,'Tatra613_V12_Wheel.glb')
bpy.ops.export_scene.gltf(
    filepath=out,
    export_format='GLB',
    use_selection=True,
    export_apply=True)
print('TATRA_V12_WHEEL_EXPORT_OK',out,'verts',len(obj.data.vertices))
