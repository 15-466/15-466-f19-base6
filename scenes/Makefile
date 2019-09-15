
BLENDER=../../blender-2.80-linux-glibc217-x86_64/blender

city.pnct : city.blend export-meshes.py
	$(BLENDER) --background --python export-meshes.py -- city.blend:Scene city.pnct
