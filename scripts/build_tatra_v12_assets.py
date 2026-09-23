import bpy, os
src=r"C:\Users\CheCat\Downloads\tatra_613_1975-1996\scene.gltf"
outdir=r"C:\Users\CheCat\Downloads\tatra_613_1975-1996\v12_clean"
os.makedirs(outdir,exist_ok=True)
bpy.ops.wm.read_factory_settings(use_empty=True)
bpy.ops.import_scene.gltf(filepath=src)
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
def export_group(names, filename, origin_mode='WORLD'):
 bpy.ops.object.select_all(action='DESELECT')
 chosen=[]
 for n in names:
  o=bpy.data.objects.get(n)
  if o: chosen += descendants(o)
 chosen=list(dict.fromkeys(chosen))
 for o in chosen: o.select_set(True)
 if not chosen: raise RuntimeError('no objects for '+filename)
 bpy.context.view_layer.objects.active=chosen[0]
 bpy.ops.object.join()
 obj=bpy.context.view_layer.objects.active
 obj.name=os.path.splitext(filename)[0]
 if origin_mode=='WORLD':
  bpy.context.scene.cursor.location=(0,0,0)
  bpy.ops.object.origin_set(type='ORIGIN_CURSOR')
 else:
  bpy.ops.object.origin_set(type='ORIGIN_GEOMETRY', center='BOUNDS')
 bpy.ops.export_scene.gltf(filepath=os.path.join(outdir,filename), export_format='GLB', use_selection=True, export_apply=True)
 print('EXPORTED',filename,'verts',len(obj.data.vertices))

export_group(base_names,'Tatra613_V12_Body.glb','WORLD')
export_group({'t613_steer'},'Tatra613_V12_Steering.glb','GEOMETRY')
export_group({'wheel.002','wheel.001','wheel'},'Tatra613_V12_Wheel.glb','GEOMETRY')
# Keep shifter visual from current cockpit until archive shifter is isolated cleanly.
print('V12_CLEAN_DONE',outdir)
