#pragma once

#include "GL.hpp"
#include "Load.hpp"

#include "Scene.hpp"

//Shader program that provides various modes for visualizing positions,
// colors, normals, and texture coordinates; mostly useful for debugging.
struct DEBUG_InspectionProgram {
	DEBUG_InspectionProgram();
	~DEBUG_InspectionProgram();

	GLuint program = 0;

	//Attribute (per-vertex variable) locations:
	GLuint Position_vec4 = -1U;
	GLuint Normal_vec3 = -1U;
	GLuint Color_vec4 = -1U;
	GLuint TexCoord_vec2 = -1U;

	//Uniform (per-invocation variable) locations:
	GLuint OBJECT_TO_CLIP_mat4 = -1U;
	GLuint OBJECT_TO_LIGHT_mat4x3 = -1U;
	GLuint NORMAL_TO_LIGHT_mat3 = -1U;

	GLuint INSPECT_MODE_int = -1U; //0: basic lighting; 1: position only; 2: normal only; 3: color only; 4: texcoord only

	//Textures:
	//no textures used
};

extern Load< DEBUG_InspectionProgram > DEBUG_inspection_program;
extern Scene::Drawable::Pipeline DEBUG_inspection_program_pipeline; //Drawable::Pipeline already initialized with proper uniform locations for this program.
