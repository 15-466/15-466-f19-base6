#pragma once

#include "GL.hpp"
#include "Load.hpp"
#include "Scene.hpp"

//Shader program that draws transformed, lit, textured vertices tinted with vertex colors:
//This is the first half -- it stores position, [normal,roughness], albedo to framebuffers:
struct BasicMaterialDeferredObjectProgram {
	BasicMaterialDeferredObjectProgram();
	~BasicMaterialDeferredObjectProgram();

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

	//Textures:
	//TEXTURE0 - texture that is accessed by TexCoord

	//Framebuffers:
	//0 - position
	//1 - (normal, roughness)
	//2 - albedo
};

extern Load< BasicMaterialDeferredObjectProgram > basic_material_deferred_object_program;

//For convenient scene-graph setup, copy this object:
// NOTE: by default, has texture bound to 1-pixel white texture -- so it's okay to use with vertex-color-only meshes.
extern Scene::Drawable::Pipeline basic_material_deferred_object_program_pipeline;

//Shader program that draws transformed, lit, textured vertices tinted with vertex colors:
//This is the second half, it reads gbuffers and applies lighting:
struct BasicMaterialDeferredLightProgram {
	BasicMaterialDeferredLightProgram();
	~BasicMaterialDeferredLightProgram();

	GLuint program = 0;

	//Attribute (per-vertex variable) locations:
	GLuint Position_vec4 = -1U;

	//Uniform (per-invocation variable) locations:
	GLuint OBJECT_TO_CLIP_mat4 = -1U;
	GLuint OBJECT_TO_LIGHT_mat4x3 = -1U;

	//  lighting uniforms:
	GLuint EYE_vec3 = -1U; //camera position in lighting space

	GLuint LIGHT_TYPE_int = -1U;
	GLuint LIGHT_LOCATION_vec3 = -1U;
	GLuint LIGHT_DIRECTION_vec3 = -1U;
	GLuint LIGHT_ENERGY_vec3 = -1U;
	GLuint LIGHT_CUTOFF_float = -1U;
	
	//Textures:
	//TEXTURE0 - position (vec3)
	//TEXTURE1 - normal,roughness (vec4)
	//TEXTURE2 - albedo
};

extern Load< BasicMaterialDeferredLightProgram > basic_material_deferred_light_program;
