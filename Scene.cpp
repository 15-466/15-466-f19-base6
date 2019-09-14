#include "Scene.hpp"

#include "gl_errors.hpp"

#include <glm/gtc/type_ptr.hpp>

//-------------------------

glm::mat4 Scene::Transform::make_local_to_parent() const {
	return glm::mat4( //translate
		glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
		glm::vec4(position, 1.0f)
	)
	* glm::mat4_cast(rotation) //rotate
	* glm::mat4( //scale
		glm::vec4(scale.x, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, scale.y, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, scale.z, 0.0f),
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	);
}

glm::mat4 Scene::Transform::make_parent_to_local() const {
	glm::vec3 inv_scale;
	inv_scale.x = (scale.x == 0.0f ? 0.0f : 1.0f / scale.x);
	inv_scale.y = (scale.y == 0.0f ? 0.0f : 1.0f / scale.y);
	inv_scale.z = (scale.z == 0.0f ? 0.0f : 1.0f / scale.z);
	return glm::mat4( //un-scale
		glm::vec4(inv_scale.x, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, inv_scale.y, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, inv_scale.z, 0.0f),
		glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)
	)
	* glm::mat4_cast(glm::inverse(rotation)) //un-rotate
	* glm::mat4( //un-translate
		glm::vec4(1.0f, 0.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 1.0f, 0.0f, 0.0f),
		glm::vec4(0.0f, 0.0f, 1.0f, 0.0f),
		glm::vec4(-position, 1.0f)
	);
}

glm::mat4 Scene::Transform::make_local_to_world() const {
	if (!parent) {
		return make_local_to_parent();
	} else {
		return parent->make_local_to_world() * make_local_to_parent();
	}
}
glm::mat4 Scene::Transform::make_world_to_local() const {
	if (!parent) {
		return make_parent_to_local();
	} else {
		return make_parent_to_local() * parent->make_world_to_local();
	}
}

//-------------------------

glm::mat4 Scene::Camera::make_projection() const {
	return glm::infinitePerspective( fovy, aspect, near );
}

//-------------------------

void Scene::draw(Camera const &camera) const {
	assert(camera.transform);
	glm::mat4 world_to_clip = camera.make_projection() * camera.transform->make_world_to_local();
	glm::mat4x3 world_to_light = glm::mat4x3(1.0f);
	draw(world_to_clip, world_to_light);
}

void Scene::draw(glm::mat4 const &world_to_clip, glm::mat4x3 const &world_to_light) const {

	//Iterate through all drawables, sending each one to OpenGL:
	for (auto const &drawable : drawables) {
		//Reference to drawable's pipeline for convenience:
		Scene::Drawable::Pipeline const &pipeline = drawable.pipeline;

		//skip any drawables without a shader program set:
		if (pipeline.program == 0) continue;
		//skip any drawables that don't contain any vertices:
		if (pipeline.count == 0) continue;


		//Set shader program:
		glUseProgram(pipeline.program);

		//Set attribute sources:
		glBindVertexArray(pipeline.vao);

		//Configure program uniforms:

		//the object-to-world matrix is used in all three of these uniforms:
		assert(drawable.transform); //drawables *must* have a transform
		glm::mat4 object_to_world = drawable.transform->make_local_to_world();

		//OBJECT_TO_CLIP takes vertices from object space to clip space:
		if (pipeline.OBJECT_TO_CLIP_mat4 != -1U) {
			glm::mat4 object_to_clip = world_to_clip * object_to_world;
			glUniformMatrix4fv(pipeline.OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(object_to_clip));
		}

		//the object-to-light matrix is used in the next two uniforms:
		glm::mat4x3 object_to_light = world_to_light * object_to_world;

		//OBJECT_TO_CLIP takes vertices from object space to light space:
		if (pipeline.OBJECT_TO_LIGHT_mat4x3 != -1U) {
			glUniformMatrix4x3fv(pipeline.OBJECT_TO_LIGHT_mat4x3, 1, GL_FALSE, glm::value_ptr(object_to_light));
		}

		//NORMAL_TO_CLIP takes normals from object space to light space:
		if (pipeline.NORMAL_TO_LIGHT_mat3 != -1U) {
			glm::mat3 normal_to_light = glm::inverse(glm::transpose(glm::mat3(object_to_light)));
			glUniformMatrix3fv(pipeline.NORMAL_TO_LIGHT_mat3, 1, GL_FALSE, glm::value_ptr(normal_to_light));
		}

		//set any requested custom uniforms:
		if (pipeline.set_uniforms) pipeline.set_uniforms();

		//set up textures:
		for (uint32_t i = 0; i < Drawable::Pipeline::TextureCount; ++i) {
			if (pipeline.textures[i].texture != 0) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(pipeline.textures[i].target, pipeline.textures[i].texture);
			}
		}

		//draw the object:
		glDrawArrays(pipeline.type, pipeline.start, pipeline.count);

		//un-bind textures:
		for (uint32_t i = 0; i < Drawable::Pipeline::TextureCount; ++i) {
			if (pipeline.textures[i].texture != 0) {
				glActiveTexture(GL_TEXTURE0 + i);
				glBindTexture(pipeline.textures[i].target, 0);
			}
		}
		glActiveTexture(GL_TEXTURE0);

	}

	glUseProgram(0);
	glBindVertexArray(0);

	GL_ERRORS();
}


//-------------------------
