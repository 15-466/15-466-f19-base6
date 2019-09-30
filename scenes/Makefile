

OS := $(shell uname)

ifeq ($(OS), Linux)
	BLENDER=../../blender-2.80-linux-glibc217-x86_64/blender
else
	BLENDER=/Applications/Blender.app/Contents/MacOS/Blender
endif

all : \
	../dist/pool.pnct \
	../dist/pool.scene \


../dist/pool.pnct : pool.blend export-meshes.py
	$(BLENDER) --background --python export-meshes.py -- 'pool.blend:Collection 1' '$@'

../dist/pool.scene : pool.blend export-scene.py
	$(BLENDER) --background --python export-scene.py -- 'pool.blend:Collection 1' '$@'


#../dist/roll-parts.pnct : sphere-roller.blend export-meshes.py
#	$(BLENDER) --background --python export-meshes.py -- sphere-roller.blend:Parts '$@'

#../dist/roll-level-1.scene : sphere-roller.blend export-scene.py
#	$(BLENDER) --background --python export-scene.py -- sphere-roller.blend:Level.1 '$@'


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
