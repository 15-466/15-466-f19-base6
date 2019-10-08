

OS := $(shell uname)

ifeq ($(OS), Linux)
	BLENDER=../../blender-2.80-linux-glibc217-x86_64/blender
else
	BLENDER=/Applications/Blender.app/Contents/MacOS/Blender
endif

all : \
	../dist/plant.pnct \
	../dist/plant.banims


../dist/plant.pnct : plant.blend export-meshes.py
	$(BLENDER) --background --python export-meshes.py -- plant.blend '$@'

../dist/plant.banims : plant.blend export-bone-animations.py
	$(BLENDER) --background --python export-bone-animations.py -- '$<' 'Plant' '[0,30]Wind;[100,140]Walk' '$@'

#../dist/pool.scene : pool.blend export-scene.py
#	$(BLENDER) --background --python export-scene.py -- 'pool.blend:Table 1' '$@'

#hexapod.pnct : hexapod.blend export-meshes.py
#	$(BLENDER) --background --python export-meshes.py -- hexapod.blend:Main '$@'
#hexapod.scene : hexapod.blend export-scene.py
#	$(BLENDER) --background --python export-scene.py -- hexapod.blend:Main '$@'
