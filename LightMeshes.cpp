#include "LightMeshes.hpp"

Load< LightMeshes > light_meshes(LoadTagEarly);

LightMeshes::LightMeshes() {

	std::vector< glm::vec4 > attribs;

	{ //'everything' cube:
		everything.type = GL_TRIANGLE_STRIP;
		everything.start = GLuint(attribs.size());

		attribs.emplace_back(-1.0f,-1.0f, 1.0f, 1.0f);
		attribs.emplace_back(-1.0f,-1.0f,-1.0f, 1.0f);
		attribs.emplace_back( 1.0f,-1.0f, 1.0f, 1.0f);
		attribs.emplace_back( 1.0f,-1.0f,-1.0f, 1.0f);
		attribs.emplace_back( 1.0f, 1.0f, 1.0f, 1.0f);
		attribs.emplace_back( 1.0f, 1.0f,-1.0f, 1.0f);
		attribs.emplace_back(-1.0f, 1.0f, 1.0f, 1.0f);
		attribs.emplace_back(-1.0f, 1.0f,-1.0f, 1.0f);
		attribs.emplace_back(-1.0f,-1.0f, 1.0f, 1.0f);
		attribs.emplace_back(-1.0f,-1.0f,-1.0f, 1.0f);
		
		attribs.emplace_back(attribs.back());
		attribs.emplace_back(-1.0f,-1.0f, 1.0f, 1.0f);
		attribs.emplace_back(attribs.back());
		attribs.emplace_back( 1.0f,-1.0f, 1.0f, 1.0f);
		attribs.emplace_back(-1.0f, 1.0f, 1.0f, 1.0f);
		attribs.emplace_back( 1.0f, 1.0f, 1.0f, 1.0f);

		attribs.emplace_back(attribs.back());
		attribs.emplace_back(-1.0f, 1.0f,-1.0f, 1.0f);
		attribs.emplace_back(attribs.back());
		attribs.emplace_back( 1.0f, 1.0f,-1.0f, 1.0f);
		attribs.emplace_back(-1.0f,-1.0f,-1.0f, 1.0f);
		attribs.emplace_back( 1.0f,-1.0f,-1.0f, 1.0f);


		everything.count = GLuint(attribs.size() - everything.start);
	}

	sphere = everything;
	cone = everything;

	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, attribs.size() * sizeof(glm::vec4), attribs.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	Position = Attrib(buffer, 4, GL_FLOAT, Attrib::AsFloat, sizeof(glm::vec4), 0);
}

LightMeshes::~LightMeshes() {
	if (buffer) {
		glDeleteBuffers(1, &buffer);
		buffer = 0;
	}
}

GLuint LightMeshes::make_vao_for_program(GLuint program) const {
	std::map< std::string, Attrib const * > attribs;

	attribs["Position"] = &Position;

	return ::make_vao_for_program(attribs, program);
}
