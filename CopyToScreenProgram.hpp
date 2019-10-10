#pragma once

#include "Load.hpp"


#include "GL.hpp"
#include "Load.hpp"
#include "Scene.hpp"
#include "Mesh.hpp"

//Shader program that copies texture data, pixel-for-pixel, to the screen:
struct CopyToScreenProgram {
	CopyToScreenProgram();
	~CopyToScreenProgram();

	GLuint program = 0;

	//Attribute (per-vertex variable) locations:
	//None! Just draw three vertices as GL_TRIANGLES

	//Uniform (per-invocation variable) locations:
	//None!
	
	//Textures:
	//TEXTURE0 - texture that is copied to the screen
};

extern GLuint empty_vao;
extern Load< CopyToScreenProgram > copy_to_screen_program;
