import bpy
import json
import os

PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(PROJECT_ROOT, "Saved", "Tatra613DesktopExport")
os.makedirs(OUT, exist_ok=True)

GLASS_NAMES = {
    o.name for o in bpy.context.scene.objects
    if o.type == 'MESH' and 't613_glass_material' in o.name.lower()
}

glass = bpy.data.materials.get('PC_T613_GlassCheap') or bpy.data.materials.new('PC_T613_GlassCheap')
glass.diffuse_color = (0.04, 0.055, 0.065, 0.15)
glass.use_nodes = True
glass.surface_render_method = 'BLENDED'
bsdf = glass.node_tree.nodes.get('Principled BSDF')
if bsdf:
    bsdf.inputs['Base Color'].default_value = (0.04, 0.055, 0.065, 1.0)
    bsdf.inputs['Roughness'].default_value = 0.18
    bsdf.inputs['Metallic'].default_value = 0.0
    bsdf.inputs['Alpha'].default_value = 0.15

all_source_meshes = [o for o in bpy.context.scene.objects if o.type == 'MESH']
# One 4-vertex low-fuel orphan sits metres outside the car. The real dashboard
# indicator is lowfuel_material.002 and remains untouched.
source_meshes = [o for o in all_source_meshes if o.name != 'lowfuel_material']
copies = []
for src in source_meshes:
    dup = src.copy()
    dup.data = src.data.copy()
    bpy.context.scene.collection.objects.link(dup)

    # Preserve the Blender scene exactly: same object transform, same geometry.
    dup.matrix_world = src.matrix_world.copy()

    # The only visual override requested for the source model: its six window objects.
    if src.name in GLASS_NAMES:
        dup.data.materials.clear()
        dup.data.materials.append(glass)

    copies.append(dup)

if len(copies) != len(source_meshes):
    raise RuntimeError('Faithful Tatra export lost source meshes')

bpy.ops.object.select_all(action='DESELECT')
for obj in copies:
    obj.select_set(True)
bpy.context.view_layer.objects.active = copies[0]
bpy.ops.object.join()
body = bpy.context.view_layer.objects.active
body.name = 'Tatra613_Desktop_Body'

filepath = os.path.join(OUT, 'Tatra613_Desktop_Body.glb')
bpy.ops.export_scene.gltf(
    filepath=filepath,
    export_format='GLB',
    use_selection=True,
    export_apply=False)

report = {
    'source': bpy.data.filepath,
    'source_mesh_count': len(all_source_meshes),
    'included_mesh_count': len(copies),
    'excluded_noncar_orphan': 'lowfuel_material',
    'glass_objects': sorted(GLASS_NAMES),
    'joined_vertex_count': len(body.data.vertices),
    'materials': [m.name if m else None for m in body.data.materials],
    'body_dimensions_m': list(body.dimensions),
    'body_location_m': list(body.matrix_world.translation),
    'body_scale': list(body.scale),
}
with open(os.path.join(OUT, 'export_report.json'), 'w', encoding='utf-8') as f:
    json.dump(report, f, indent=2)

print('DESKTOP_TATRA_FAITHFUL_EXPORT_OK', filepath)
print(json.dumps(report, indent=2))
