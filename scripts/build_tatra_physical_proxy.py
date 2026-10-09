"""Build an invisible Tatra collision/rig source; never edit the accepted visual source.
Run with Blender --background --python this_file -- --source SRC --output DIR.
The approved visual X transform is baked into this new physical proxy only.
"""
import argparse
import hashlib
import json
import sys
from pathlib import Path

import bmesh
import bpy
from mathutils import Vector


def hull(points):
    bm = bmesh.new()
    for point in points:
        bm.verts.new(point)
    bm.verts.ensure_lookup_table()
    result = bmesh.ops.convex_hull(bm, input=list(bm.verts), use_existing_faces=False)
    loose = set(result.get('geom_interior', [])) | set(result.get('geom_unused', []))
    if loose:
        bmesh.ops.delete(bm, geom=list(loose), context='VERTS')
    bmesh.ops.triangulate(bm, faces=list(bm.faces))
    bm.verts.ensure_lookup_table()
    bm.verts.index_update()
    vertices = [list(v.co) for v in bm.verts]
    faces = [[v.index for v in face.verts] for face in bm.faces]
    bm.free()
    if len(vertices) < 4 or len(faces) < 4:
        raise RuntimeError('Degenerate physical hull')
    return vertices, faces


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--source', required=True)
    parser.add_argument('--output', required=True)
    args = parser.parse_args(sys.argv[sys.argv.index('--') + 1:])
    source, output = Path(args.source).resolve(), Path(args.output).resolve()
    if not source.is_file():
        raise RuntimeError(f'Missing accepted source: {source}')
    if output.exists():
        raise RuntimeError(f'Output must be new; preserve existing authoring evidence: {output}')
    before = hashlib.sha256(source.read_bytes()).hexdigest()
    bpy.ops.wm.open_mainfile(filepath=str(source))
    for obj in bpy.data.objects:
        if obj.type == 'ARMATURE':
            obj.data.pose_position = 'REST'
    bpy.context.scene.frame_set(1)
    bpy.context.view_layer.update()
    settings = {'system': bpy.context.scene.unit_settings.system,
                'scale_length': bpy.context.scene.unit_settings.scale_length}
    points, names = [], []
    for obj in bpy.data.objects:
        if obj.type != 'MESH' or not obj.name.startswith('pessimat613_body_material'):
            continue
        names.append(obj.name)
        for v in obj.data.vertices:
            w = obj.matrix_world @ v.co
            points.append((0.055167 + 0.959121 * w.x, -w.y, w.z))
    if len(names) != 10 or len(points) < 100:
        raise RuntimeError(f'Unexpected RIG24 body inventory: {names}')
    # Two convex regions retain the low bonnet/deck and upper passenger cabin.
    # The split is an explicit collision approximation, not a handling coefficient.
    regions = [('LowerBody', [p for p in points if p[2] <= 0.91]),
               ('Cabin', [p for p in points if p[2] >= 0.82])]
    hulls = []
    for name, pts in regions:
        vertices, faces = hull(pts)
        hulls.append({'name': name, 'vertices_cm': [[100*x for x in v] for v in vertices],
                      'vertices_m': vertices, 'triangles': faces})
    bpy.ops.wm.read_factory_settings(use_empty=True)
    scene = bpy.context.scene
    scene.unit_settings.system = 'METRIC'
    scene.unit_settings.scale_length = 0.01  # author this proxy directly in centimetres
    arm_data = bpy.data.armatures.new('Tatra613PhysicalSkeleton')
    arm = bpy.data.objects.new('Armature', arm_data)
    scene.collection.objects.link(arm)
    bpy.context.view_layer.objects.active = arm
    arm.select_set(True)
    bpy.ops.object.mode_set(mode='EDIT')
    root = arm_data.edit_bones.new('Root')
    root.head, root.tail = (0, 0, 0), (0, 10, 0)
    wheels = {'Phys_Wheel_FL': (1.35, -0.76, 0.25),
              'Phys_Wheel_FR': (1.35, 0.76, 0.25),
              'Phys_Wheel_BL': (-1.63, -0.76, 0.265),
              'Phys_Wheel_BR': (-1.63, 0.76, 0.265)}
    for name, pos in wheels.items():
        bone = arm_data.edit_bones.new(name)
        # FBX converts Blender +Y to Unreal -Y; make that basis conversion once.
        head = (pos[0]*100, -pos[1]*100, pos[2]*100)
        bone.head, bone.tail = head, (head[0], head[1] + 5, head[2])
        bone.parent = root
    bpy.ops.object.mode_set(mode='OBJECT')
    vertices, triangles = [], []
    for region in hulls:
        start = len(vertices)
        vertices += region['vertices_m']
        triangles += [[start+i for i in face] for face in region['triangles']]
    root_indices = list(range(len(vertices)))
    wheel_indices = {}
    for name, point in wheels.items():
        start = len(vertices)
        for offset in [(0, 0, 0), (.001, 0, 0), (0, .001, 0), (0, 0, .001)]:
            vertices.append(list(Vector(point) + Vector(offset)))
        triangles += [[start+i for i in face] for face in [(0,2,1),(0,1,3),(0,3,2),(1,2,3)]]
        wheel_indices[name] = list(range(start, start+4))
    mesh_data = bpy.data.meshes.new('Tatra613PhysicalProxy')
    mesh_data.from_pydata([(x*100, -y*100, z*100) for x,y,z in vertices], [], triangles)
    mesh_data.update()
    mesh = bpy.data.objects.new('Tatra613PhysicalProxy', mesh_data)
    scene.collection.objects.link(mesh)
    mesh.parent = arm
    mesh.vertex_groups.new(name='Root').add(root_indices, 1.0, 'REPLACE')
    for name, indices in wheel_indices.items():
        mesh.vertex_groups.new(name=name).add(indices, 1.0, 'REPLACE')
    modifier = mesh.modifiers.new('Armature', 'ARMATURE')
    modifier.object = arm
    mesh.select_set(True)
    output.mkdir(parents=True)
    fbx = output / 'Tatra613_Physical.fbx'
    bpy.ops.export_scene.fbx(filepath=str(fbx), use_selection=True,
        object_types={'ARMATURE', 'MESH'}, apply_unit_scale=True,
        apply_scale_options='FBX_SCALE_UNITS', use_mesh_modifiers=True,
        mesh_smooth_type='FACE', use_tspace=False, add_leaf_bones=False,
        bake_anim=False, axis_forward='-Y', axis_up='Z')
    bpy.ops.wm.save_as_mainfile(filepath=str(output / 'Tatra613_Physical.blend'))
    recipe = {'schema': 1, 'source': str(source), 'source_sha256': before,
              'source_objects': sorted(names), 'blender': bpy.app.version_string,
              'source_scene_units': settings, 'coordinate_frame': 'accepted_chassis_cm',
              'visual_transform_baked': {'x_scale': .959121, 'x_offset_cm': 5.5167},
              'hulls': [{k:v for k,v in h.items() if k != 'vertices_m'} for h in hulls],
              'wheel_bones_cm': {n:[v*100 for v in pos] for n,pos in wheels.items()},
              'fbx_sha256': hashlib.sha256(fbx.read_bytes()).hexdigest()}
    if hashlib.sha256(source.read_bytes()).hexdigest() != before:
        raise RuntimeError('Accepted visual source unexpectedly changed')
    (output / 'Tatra613_Physical.recipe.json').write_text(json.dumps(recipe, indent=2), encoding='utf-8')
    print('TATRA_PHYSICAL_PROXY_AUTHORED', json.dumps({'hulls':len(hulls), 'vertices':len(vertices),
          'triangles':len(triangles), 'source_unchanged':True, 'fbx':str(fbx)}))


if __name__ == '__main__':
    main()
