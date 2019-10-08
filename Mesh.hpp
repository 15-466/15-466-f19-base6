#pragma once

/*
 * In this code, "Mesh" is a range of vertices that should be sent through
 *  the OpenGL pipeline together.
 * A "MeshBuffer" holds a collection of such meshes (loaded from a file) in
 *  a single OpenGL array buffer. Individual meshes can be looked up by name
 *  using the MeshBuffer::lookup() function.
 *
 */

#include "make_vao_for_program.hpp"
#include "GL.hpp"

#include <glm/glm.hpp>
#include <map>
#include <limits>
#include <string>
#include <vector>


struct Mesh {
	//Meshes are vertex ranges (and primitive types) in their MeshBuffer:

	GLenum type = GL_TRIANGLES; //type of primitives in mesh
	GLuint start = 0; //index of first vertex
	GLuint count = 0; //count of vertices

	//Bounding box.
	//useful for debug visualization and collision detection:
	glm::vec3 min = glm::vec3( std::numeric_limits< float >::infinity());
	glm::vec3 max = glm::vec3(-std::numeric_limits< float >::infinity());
};

struct MeshBuffer {
	//construct from a file:
	// note: will throw if file fails to read.
	MeshBuffer(std::string const &filename);

	//look up a particular mesh by name:
	// note: will throw if mesh not found.
	const Mesh &lookup(std::string const &name) const;
	
	//build a vertex array object that links this vbo to attributes to a program:
	// note: will throw if program defines attributes not contained in this buffer
	GLuint make_vao_for_program(GLuint program) const;

	//This is the OpenGL vertex buffer object containing the mesh data:
	GLuint buffer = 0;

	//-- internals ---

	//used by the lookup() function:
	std::map< std::string, Mesh > meshes;

	//describe the format of the mesh:
	Attrib Position;
	Attrib Normal;
	Attrib Color;
	Attrib TexCoord;

	//local copy of vertex information: (for collision detection)
	std::vector< glm::vec3 > positions;
};
