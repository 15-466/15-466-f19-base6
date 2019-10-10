#pragma once

#include "make_vao_for_program.hpp"
#include "Mesh.hpp"
#include "Load.hpp"

struct LightMeshes {
	LightMeshes();
	~LightMeshes();

	//object with w=0 that -- thus -- covers whole scene:
	Mesh everything;

	//cube, radius-1:
	Mesh cube;
	//bounding sphere, radius-1:
	Mesh sphere;
	//height-1, radius-2 cone, vertex at (0,0), extends along -z axis:
	Mesh cone;

	//build a vertex array object that links these meshes to a given program:
	GLuint make_vao_for_program(GLuint program) const;

	//storage:
	GLuint buffer = 0;

	//storage layout:
	Attrib Position;
};

extern Load< LightMeshes > light_meshes;
