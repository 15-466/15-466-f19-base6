#pragma once

/*
 * "MeshBuffer" holds a collection of meshes loaded from a file into a single
 *   OpenGL array buffer.
 * Individual meshes correspond to ranges of vertices within the buffer, and
 *  can be looked up with the 'lookup()' method.
 */

#include "GL.hpp"
#include <glm/glm.hpp>
#include <map>
#include <limits>

struct MeshBuffer {
	GLuint buffer = 0; //OpenGL array buffer containing the mesh data

	//construct from a file:
	// note: will throw if file fails to read.
	MeshBuffer(std::string const &filename);

	//look up a particular mesh in the DB:
	// note: will throw if mesh not found.
	struct Mesh {
		GLenum type = GL_TRIANGLES; //type of primitives in mesh
		GLuint start = 0; //index of first vertex
		GLuint count = 0; //count of vertices
		//useful for debug visualization and (perhaps, eventually) collision detection:
		glm::vec3 min = glm::vec3( std::numeric_limits< float >::infinity());
		glm::vec3 max = glm::vec3(-std::numeric_limits< float >::infinity());
	};
	const Mesh &lookup(std::string const &name) const;
	
	//build a vertex array object that links this vbo to attributes to a program:
	// will throw if program defines attributes not contained in this buffer
	GLuint make_vao_for_program(GLuint program) const;

	//-- internals ---

	//used by the lookup() function:
	std::map< std::string, Mesh > meshes;

	//These 'Attrib' structures describe the location of various attributes within the buffer (in exactly format wanted by glVertexAttribPointer), and are used by the "make_vao_for_program" call:
	struct Attrib {
		GLint size = 0;
		GLenum type = 0;
		GLboolean normalized = GL_FALSE;
		GLsizei stride = 0;
		GLsizei offset = 0;

		Attrib() = default;
		Attrib(GLint size_, GLenum type_, GLboolean normalized_, GLsizei stride_, GLsizei offset_)
		: size(size_), type(type_), normalized(normalized_), stride(stride_), offset(offset_) { }
	};

	Attrib Position;
	Attrib Normal;
	Attrib Color;
	Attrib TexCoord;
};
