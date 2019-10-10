#include "DemoLightingDeferredMode.hpp"
#include "DrawLines.hpp"
#include "Load.hpp"
#include "data_path.hpp"
#include "BasicMaterialDeferredProgram.hpp"
#include "CopyToScreenProgram.hpp"
#include "LightMeshes.hpp"
#include "gl_errors.hpp"
#include "check_fb.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>

GLuint spheres_for_basic_material_deferred_object = 0;
GLuint light_for_basic_material_deferred_light = 0;
extern Load< MeshBuffer > spheres_meshes;

Load< Scene > spheres_scene_deferred(LoadTagLate, []() -> Scene const * {
	light_for_basic_material_deferred_light = light_meshes->make_vao_for_program(basic_material_deferred_light_program->program);
	spheres_for_basic_material_deferred_object = spheres_meshes->make_vao_for_program(basic_material_deferred_object_program->program);

	Scene *ret = new Scene(data_path("spheres.scene"), [](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = spheres_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable::Pipeline &pipeline = scene.drawables.back().pipeline;
		pipeline = basic_material_deferred_object_program_pipeline;
		pipeline.vao = spheres_for_basic_material_deferred_object;
		pipeline.type = mesh.type;
		pipeline.start = mesh.start;
		pipeline.count = mesh.count;

		float roughness = 1.0f;
		if (transform->name.substr(0, 9) == "Icosphere") {
			roughness = (transform->position.y + 10.0f) / 18.0f;
		}
		pipeline.set_uniforms = [roughness](){
			glUniform1f(basic_material_deferred_object_program->ROUGHNESS_float, roughness);
		};
	});

	return ret;
});



//Helper: maintain a framebuffer to hold rendered geometry
struct FB {
	//object data gets stored in these textures:
	GLuint position_tex = 0;
	GLuint normal_roughness_tex = 0;
	GLuint albedo_tex = 0;
	
	//output image gets written to this texture:
	GLuint output_tex = 0;

	//depth buffer is shared between objects + lights pass:
	GLuint depth_rb = 0;

	GLuint objects_fb = 0; //(position, normal, albedo) + depth
	GLuint lights_fb = 0; //(output) + depth

	glm::uvec2 size = glm::uvec2(0);

	void resize(glm::uvec2 const &drawable_size) {
		if (drawable_size == size) return;
		size = drawable_size;

		//helper to allocate a texture:
		auto alloc_tex = [&](GLuint &tex, GLenum internal_format) {
			if (tex == 0) glGenTextures(1, &tex);
			glBindTexture(GL_TEXTURE_2D, tex);
			glTexImage2D(GL_TEXTURE_2D, 0, internal_format, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glBindTexture(GL_TEXTURE_2D, 0);
		};

		//set up position_tex as a 32-bit floating point RGB texture:
		alloc_tex(position_tex, GL_RGB32F);

		//set up normal_roughness_tex as a 16-bit floating point RGBA texture:
		alloc_tex(normal_roughness_tex, GL_RGBA16F);

		//set up albedo_tex as an 8-bit fixed point RGBA texture:
		alloc_tex(albedo_tex, GL_RGBA8);

		//set up output_tex as an 8-bit fixed point RGBA texture:
		alloc_tex(output_tex, GL_RGBA8);

		//if depth_rb does not have a name, name it:
		if (depth_rb == 0) glGenRenderbuffers(1, &depth_rb);
		//set up depth_rb as a 24-bit fixed-point depth buffer:
		glBindRenderbuffer(GL_RENDERBUFFER, depth_rb);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, size.x, size.y);
		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		//if objects framebuffer doesn't have a name, name it and attach textures:
		if (objects_fb == 0) {
			glGenFramebuffers(1, &objects_fb);
			//set up framebuffer: (don't need to do when resizing)
			glBindFramebuffer(GL_FRAMEBUFFER, objects_fb);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, position_tex, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, normal_roughness_tex, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, albedo_tex, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);
			GLenum bufs[3] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
			glDrawBuffers(3, bufs);
			check_fb();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		//if lights-drawing framebuffer doesn't have a name, name it and attach textures:
		if (lights_fb == 0) {
			glGenFramebuffers(1, &lights_fb);
			//set up framebuffer: (don't need to do when resizing)
			glBindFramebuffer(GL_FRAMEBUFFER, lights_fb);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, output_tex, 0);
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_rb);
			GLenum bufs[1] = {GL_COLOR_ATTACHMENT0};
			glDrawBuffers(1, bufs);
			check_fb();
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

	}
} fb;



DemoLightingDeferredMode::DemoLightingDeferredMode() {
}

DemoLightingDeferredMode::~DemoLightingDeferredMode() {
}

bool DemoLightingDeferredMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_1) {
		show = ShowOutput;
	}
	if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_2) {
		show = ShowPosition;
	}
	if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_3) {
		show = ShowNormalRoughness;
	}
	if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_4) {
		show = ShowAlbedo;
	}
	return DemoLightingMultipassMode::handle_event(evt, window_size);
}

void DemoLightingDeferredMode::draw(glm::uvec2 const &drawable_size) {
	fb.resize(drawable_size);

	//--- use camera structure to set up scene camera ---

	scene_camera->transform->rotation =
		glm::angleAxis(camera.azimuth, glm::vec3(0.0f, 0.0f, 1.0f))
		* glm::angleAxis(0.5f * 3.1415926f + -camera.elevation, glm::vec3(1.0f, 0.0f, 0.0f))
	;
	scene_camera->transform->position = camera.target + camera.radius * (scene_camera->transform->rotation * glm::vec3(0.0f, 0.0f, 1.0f));
	scene_camera->transform->scale = glm::vec3(1.0f);
	scene_camera->aspect = float(drawable_size.x) / float(drawable_size.y);


	glm::mat4 world_to_clip = scene_camera->make_projection() * scene_camera->transform->make_world_to_local();
	glm::vec3 eye = scene_camera->transform->make_local_to_world()[3];
	
	//--- draw geometry to framebuffer ---

	glBindFramebuffer(GL_FRAMEBUFFER, fb.objects_fb);

	GLfloat zeros[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	glClearBufferfv(GL_COLOR, 0, zeros);
	glClearBufferfv(GL_COLOR, 1, zeros);
	glClearBufferfv(GL_COLOR, 2, zeros);
	glClear(GL_DEPTH_BUFFER_BIT);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//draw objects to geometry framebuffers:
	spheres_scene_deferred->draw(world_to_clip);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GL_ERRORS();

	//--- draw lights, reading geometry from framebuffer ---

	glBindFramebuffer(GL_FRAMEBUFFER, fb.lights_fb);

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glDepthFunc(GL_GREATER);

	glCullFace(GL_FRONT);
	glEnable(GL_CULL_FACE);

	glEnable(GL_BLEND);
	glBlendEquation(GL_FUNC_ADD);
	glBlendFunc(GL_ONE, GL_ONE);
	glDepthMask(GL_FALSE);

	//draw geometry for each light:
	auto &prog = basic_material_deferred_light_program;
	glUseProgram(prog->program);

	glBindVertexArray(light_for_basic_material_deferred_light);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, fb.position_tex);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, fb.normal_roughness_tex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, fb.albedo_tex);

	for (auto const &light : spheres_scene_deferred->lights) {
		glm::mat4 light_to_world = light.transform->make_local_to_world();

		Mesh const *mesh = nullptr;

		glUniform3fv(prog->EYE_vec3, 1, glm::value_ptr(eye));
		glUniform3fv(prog->LIGHT_LOCATION_vec3, 1, glm::value_ptr(glm::vec3(light_to_world[3])));
		glUniform3fv(prog->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(-light_to_world[2])));
		glUniform3fv(prog->LIGHT_ENERGY_vec3, 1, glm::value_ptr(light.energy));
		if (light.type == Scene::Light::Point) {
			glUniform1i(prog->LIGHT_TYPE_int, 0);
			glUniform1f(prog->LIGHT_CUTOFF_float, 1.0f);
			mesh = &light_meshes->cube;
			//when is energy / dis^2 < 1/256.0f?
			float R = std::sqrt(256.0f * std::max(light.energy.x, std::max(light.energy.y, light.energy.z)));
			light_to_world = light_to_world * glm::mat4(
				R, 0.0f, 0.0f, 0.0f,
				0.0f, R, 0.0f, 0.0f,
				0.0f, 0.0f, R, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);
		} else if (light.type == Scene::Light::Hemisphere) {
			glUniform1i(prog->LIGHT_TYPE_int, 1);
			glUniform1f(prog->LIGHT_CUTOFF_float, 1.0f);
			mesh = &light_meshes->everything;
			float R = 1.0f;
			light_to_world = light_to_world * glm::mat4(
				R, 0.0f, 0.0f, 0.0f,
				0.0f, R, 0.0f, 0.0f,
				0.0f, 0.0f, R, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);
		} else if (light.type == Scene::Light::Spot) {
			glUniform1i(prog->LIGHT_TYPE_int, 2);
			glUniform1f(prog->LIGHT_CUTOFF_float, std::cos(0.5f * light.spot_fov));
			mesh = &light_meshes->cone;
			float R = std::sqrt(256.0f * std::max(light.energy.x, std::max(light.energy.y, light.energy.z)));
			//HACK: hard-limit to 5 units:
			R = 5.0f;
			float C = std::tan(0.5f * light.spot_fov);
			light_to_world = light_to_world * glm::mat4(
				C*R, 0.0f, 0.0f, 0.0f,
				0.0f, C*R, 0.0f, 0.0f,
				0.0f, 0.0f, R, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);
		} else if (light.type == Scene::Light::Directional) {
			glUniform1i(prog->LIGHT_TYPE_int, 3);
			glUniform1f(prog->LIGHT_CUTOFF_float, 1.0f);
			mesh = &light_meshes->everything;
			float R = 1.0f;
			light_to_world = light_to_world * glm::mat4(
				R, 0.0f, 0.0f, 0.0f,
				0.0f, R, 0.0f, 0.0f,
				0.0f, 0.0f, R, 0.0f,
				0.0f, 0.0f, 0.0f, 1.0f
			);
		}
		glUniformMatrix4fv(prog->OBJECT_TO_CLIP_mat4, 1, GL_FALSE, glm::value_ptr(world_to_clip * light_to_world));

		if (mesh && mesh->count) {
			glDrawArrays(mesh->type, mesh->start, mesh->count);
		}
	}

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindVertexArray(0);

	glDepthMask(GL_TRUE);

	glDisable(GL_CULL_FACE);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GL_ERRORS();

	//--- copy lights fb info to screen ---
	
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	GL_ERRORS();
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	GL_ERRORS();

	glBindVertexArray(empty_vao);
	glUseProgram(copy_to_screen_program->program);

	glActiveTexture(GL_TEXTURE0);
	if (show == ShowOutput) {
		glBindTexture(GL_TEXTURE_2D, fb.output_tex);
	} else if (show == ShowPosition) {
		glBindTexture(GL_TEXTURE_2D, fb.position_tex);
	} else if (show == ShowNormalRoughness) {
		glBindTexture(GL_TEXTURE_2D, fb.normal_roughness_tex);
	} else if (show == ShowAlbedo) {
		glBindTexture(GL_TEXTURE_2D, fb.albedo_tex);
	}

	GL_ERRORS();
	glDrawArrays(GL_TRIANGLES, 0, 3);
	GL_ERRORS();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, 0);

	glBindVertexArray(0);
	glUseProgram(0);


	GL_ERRORS();


}
