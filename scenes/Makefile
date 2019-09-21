
BLENDER=../../blender-2.80-linux-glibc217-x86_64/blender

all : \
	../dist/roll-parts.pnct \
	../dist/roll-level-1.scene \
	../dist/roll-level-2.scene \
	../dist/roll-level-3.scene \


../dist/roll-parts.pnct : sphere-roller.blend export-meshes.py
	$(BLENDER) --background --python export-meshes.py -- sphere-roller.blend:Parts '$@'

../dist/roll-level-1.scene : sphere-roller.blend export-scene.py
	$(BLENDER) --background --python export-scene.py -- sphere-roller.blend:Level.1 '$@'

../dist/roll-level-2.scene : sphere-roller.blend export-scene.py
	$(BLENDER) --background --python export-scene.py -- sphere-roller.blend:Level.2 '$@'

../dist/roll-level-3.scene : sphere-roller.blend export-scene.py
	$(BLENDER) --background --python export-scene.py -- sphere-roller.blend:Level.3 '$@'

#../dist/city.scene : city.blend export-scene.py
#	$(BLENDER) --background --python export-scene.py -- city.blend:Scene '$@'
#../dist/brunch.pnct : brunch.blend export-meshes.py
#	$(BLENDER) --background --python export-meshes.py -- brunch.blend '$@'
#../dist/brunch.scene : brunch.blend export-scene.py
#	$(BLENDER) --background --python export-scene.py -- brunch.blend '$@'

#../dist/garden.pnct : garden.blend export-meshes.py
#	$(BLENDER) --background --python export-meshes.py -- garden.blend '$@'
#../dist/garden.scene : garden.blend export-scene.py
#	$(BLENDER) --background --python export-scene.py -- garden.blend '$@'

#hexapod.pnct : hexapod.blend export-meshes.py
#	$(BLENDER) --background --python export-meshes.py -- hexapod.blend:Main '$@'
#hexapod.scene : hexapod.blend export-scene.py
#	$(BLENDER) --background --python export-scene.py -- hexapod.blend:Main '$@'
