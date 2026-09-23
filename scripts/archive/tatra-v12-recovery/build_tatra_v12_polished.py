import bpy, os
from mathutils import Vector, Matrix
src=os.environ.get("PINKCAB_TATRA_SOURCE_GLTF")
outdir=os.environ.get("PINKCAB_TATRA_V12_CLEAN_DIR")
if not src or not outdir:
    raise RuntimeError("PINKCAB_TATRA_SOURCE_GLTF and PINKCAB_TATRA_V12_CLEAN_DIR are required")
os.makedirs(outdir,exist_ok=True)
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=src)

def set_pbr(mat, color, metallic=0.0, roughness=0.55):
    mat.diffuse_color=(*color,1.0)
    if mat.use_nodes and mat.node_tree:
        bsdf=next((n for n in mat.node_tree.nodes if n.type=='BSDF_PRINCIPLED'),None)
        if bsdf:
            bsdf.inputs['Base Color'].default_value=(*color,1.0)
            if 'Metallic' in bsdf.inputs: bsdf.inputs['Metallic'].default_value=metallic
            bsdf.inputs['Roughness'].default_value=roughness

for mat in bpy.data.materials:
    n=mat.name.lower()
    if 'pessimat613_body' in n or 't613_color' in n:
        set_pbr(mat,(0.95,0.01,0.20),0.08,0.30)
    elif 't613_black' in n:
        set_pbr(mat,(0.012,0.014,0.018),0.0,0.48)
    elif any(k in n for k in ('interiorspec','int1','int2','interior__spec')):
        set_pbr(mat,(0.035,0.040,0.050),0.0,0.62)
    elif 'leatherspec' in n:
        set_pbr(mat,(0.020,0.022,0.026),0.0,0.82)
    elif 't613_chrome' in n:
        set_pbr(mat,(0.30,0.32,0.35),0.9,0.18)

base_names={
 't613_headlin','t613_dash','t613_body.001','t613_seat_FL','t613_seat_FR','t613_seats',
 't613_taillight_L','t613_taillight_R','t613_mirror_R','t613_mirror_L','t613_fender_L','t613_fender_R',
 't613_mudsli_F','t613_mudsli_R','t613_radsupport','t613_headlight_L','t613_headlight_R','t613_engine',
 't613_doorglass_FR','t613_doorglass_RR','t613_doorglass_RL','t613_doorglass_FL','t613_windshield','bottom.002',
 't613_bumper_R','t613_bumper_F','t613_door_FL','t613_door_FR','t613_door_RL','t613_door_RR','t613_exhaust',
 't613_backlight','t613_trunk','t613_hood'}
def descendants(root):
    out=[]
    def rec(o):
        if o.type=='MESH': out.append(o)
        for c in o.children: rec(c)
    rec(root); return out

def select_join(names):
    bpy.ops.object.select_all(action='DESELECT')
    chosen=[]
    for n in names:
        o=bpy.data.objects.get(n)
        if o: chosen += descendants(o)
    chosen=list(dict.fromkeys(chosen))
    for o in chosen: o.select_set(True)
    if not chosen: raise RuntimeError('empty selection')
    bpy.context.view_layer.objects.active=chosen[0]
    if len(chosen)>1: bpy.ops.object.join()
    return bpy.context.view_layer.objects.active

def export_body():
    obj=select_join(base_names); obj.name='Tatra613_V12_Body'
    bpy.context.scene.cursor.location=(0,0,0); bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
    bpy.ops.export_scene.gltf(filepath=os.path.join(outdir,'Tatra613_V12_Body.glb'),export_format='GLB',use_selection=True,export_apply=True)
def export_centered(names, filename):
    obj=select_join(names)
    bpy.ops.object.transform_apply(location=True,rotation=True,scale=True)
    pts=[Vector(c) for c in obj.bound_box]
    center=sum(pts,Vector())/8.0
    obj.data.transform(Matrix.Translation(-center))
    obj.location=(0,0,0); obj.rotation_euler=(0,0,0); obj.scale=(1,1,1)
    obj.name=os.path.splitext(filename)[0]
    bpy.ops.export_scene.gltf(filepath=os.path.join(outdir,filename),export_format='GLB',use_selection=True,export_apply=True)

export_body()
# Reload source between exports so joining the body does not consume control objects.
bpy.ops.wm.read_factory_settings(use_empty=True); bpy.ops.import_scene.gltf(filepath=src)
for mat in bpy.data.materials:
    n=mat.name.lower()
    if 't613_black' in n: set_pbr(mat,(0.012,0.014,0.018),0.0,0.48)
    elif 't613_chrome' in n: set_pbr(mat,(0.30,0.32,0.35),0.9,0.18)
export_centered({'t613_steer'},'Tatra613_V12_Steering.glb')
bpy.ops.wm.read_factory_settings(use_empty=True); bpy.ops.import_scene.gltf(filepath=src)
for mat in bpy.data.materials:
    n=mat.name.lower()
    if 't613_black' in n: set_pbr(mat,(0.012,0.014,0.018),0.0,0.48)
    elif 't613_chrome' in n: set_pbr(mat,(0.30,0.32,0.35),0.9,0.18)
export_centered({'wheel.002','wheel.001','wheel'},'Tatra613_V12_Wheel.glb')
print('V12_POLISHED_DONE')
