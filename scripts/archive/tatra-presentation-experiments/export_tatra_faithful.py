import bpy, os, json
from mathutils import Vector

PROJECT_ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(PROJECT_ROOT, "Saved", "Tatra613FaithfulExport")
os.makedirs(OUT, exist_ok=True)

SOURCE_MESHES = [o for o in bpy.context.scene.objects if o.type == 'MESH']
SOURCE_VERTS = sum(len(o.data.vertices) for o in SOURCE_MESHES)
SOURCE_POLYS = sum(len(o.data.polygons) for o in SOURCE_MESHES)

def scene_bounds(objects):
    pts = [o.matrix_world @ Vector(c) for o in objects for c in o.bound_box]
    return (
        [min(p[i] for p in pts) for i in range(3)],
        [max(p[i] for p in pts) for i in range(3)],
    )

SOURCE_MIN, SOURCE_MAX = scene_bounds(SOURCE_MESHES)

glass_names = {o.name for o in SOURCE_MESHES if o.name.startswith('t613_glass_material')}
glass_slot = bpy.data.materials.get('PC_T613_GlassSlot') or bpy.data.materials.new('PC_T613_GlassSlot')
glass_slot.use_nodes = True
glass_slot.diffuse_color = (0.05, 0.07, 0.09, 0.16)
glass_bsdf = glass_slot.node_tree.nodes.get('Principled BSDF')
if glass_bsdf:
    glass_bsdf.inputs['Base Color'].default_value = (0.05, 0.07, 0.09, 1.0)
    glass_bsdf.inputs['Roughness'].default_value = 0.20
    glass_bsdf.inputs['Metallic'].default_value = 0.0
    glass_bsdf.inputs['Alpha'].default_value = 0.16

copies = []
for src in SOURCE_MESHES:
    dup = src.copy()
    dup.data = src.data.copy()
    bpy.context.scene.collection.objects.link(dup)
    dup.matrix_world = src.matrix_world.copy()

    # The only authored visual change: mark the six existing glass objects
    # with a unique slot so Unreal can make them transparent after import.
    if src.name in glass_names:
        dup.data.materials.clear()
        dup.data.materials.append(glass_slot)

    bpy.ops.object.select_all(action='DESELECT')
    dup.select_set(True)
    bpy.context.view_layer.objects.active = dup
    bpy.ops.object.transform_apply(location=True, rotation=True, scale=True)
    copies.append(dup)

bpy.ops.object.select_all(action='DESELECT')
for obj in copies:
    obj.select_set(True)
bpy.context.view_layer.objects.active = copies[0]
bpy.ops.object.join()
body = bpy.context.view_layer.objects.active
body.name = 'Tatra613_FaithfulBody'

OUT_VERTS = len(body.data.vertices)
OUT_POLYS = len(body.data.polygons)
bounds = [body.matrix_world @ Vector(c) for c in body.bound_box]
OUT_MIN = [min(v[i] for v in bounds) for i in range(3)]
OUT_MAX = [max(v[i] for v in bounds) for i in range(3)]

if OUT_VERTS != SOURCE_VERTS:
    raise RuntimeError(f'Vertex count changed: source={SOURCE_VERTS} export={OUT_VERTS}')
if OUT_POLYS != SOURCE_POLYS:
    raise RuntimeError(f'Polygon count changed: source={SOURCE_POLYS} export={OUT_POLYS}')
for a, b in zip(SOURCE_MIN + SOURCE_MAX, OUT_MIN + OUT_MAX):
    if abs(a - b) > 1e-5:
        raise RuntimeError(f'Bounds changed: source={SOURCE_MIN,SOURCE_MAX} export={OUT_MIN,OUT_MAX}')

bpy.ops.export_scene.gltf(
    filepath=os.path.join(OUT, 'Tatra613_FaithfulBody.glb'),
    export_format='GLB',
    use_selection=True,
    export_apply=False)

report = {
    'source': bpy.data.filepath,
    'source_mesh_count': len(SOURCE_MESHES),
    'source_vertices': SOURCE_VERTS,
    'source_polygons': SOURCE_POLYS,
    'source_bounds_min': SOURCE_MIN,
    'source_bounds_max': SOURCE_MAX,
    'export_vertices': OUT_VERTS,
    'export_polygons': OUT_POLYS,
    'export_bounds_min': OUT_MIN,
    'export_bounds_max': OUT_MAX,
    'glass_objects': sorted(glass_names),
    'material_slots': [m.name if m else None for m in body.data.materials],
}
with open(os.path.join(OUT, 'export_report.json'), 'w', encoding='utf-8') as f:
    json.dump(report, f, indent=2)

print('TATRA_FAITHFUL_EXPORT_OK')
print(json.dumps(report, indent=2))
