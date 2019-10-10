#pragma once

#include "GL.hpp"
#include "Load.hpp"
#include "Scene.hpp"

//Shader program that draws transformed, lit, textured vertices tinted with vertex colors:
struct BasicMaterialForwardProgram {
	BasicMaterialForwardProgram();
	~BasicMaterialForwardProgram();

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

	//  material uniforms:
	GLuint ROUGHNESS_float = -1U;

	//  lighting uniforms:
	GLuint EYE_vec3 = -1U; //camera position in lighting space

	GLuint LIGHTS_uint = -1U;

	GLuint LIGHT_TYPE_int_array = -1U;
	GLuint LIGHT_LOCATION_vec3_array = -1U;
	GLuint LIGHT_DIRECTION_vec3_array = -1U;
	GLuint LIGHT_ENERGY_vec3_array = -1U;
	GLuint LIGHT_CUTOFF_float_array = -1U;

	enum : uint32_t { MaxLights = 40 };
	
	//Textures:
	//TEXTURE0 - texture that is accessed by TexCoord
};

extern Load< BasicMaterialForwardProgram > basic_material_forward_program;

//For convenient scene-graph setup, copy this object:
// NOTE: by default, has texture bound to 1-pixel white texture -- so it's okay to use with vertex-color-only meshes.
extern Scene::Drawable::Pipeline basic_material_forward_program_pipeline;
