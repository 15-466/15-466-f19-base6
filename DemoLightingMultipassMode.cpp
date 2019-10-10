#include "DemoLightingMultipassMode.hpp"
#include "DrawLines.hpp"
#include "Load.hpp"
#include "data_path.hpp"
#include "demo_menu.hpp"
#include "BasicMaterialProgram.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>

GLuint spheres_for_basic_material = -1U;

Load< MeshBuffer > spheres_meshes(LoadTagDefault, []() -> MeshBuffer const * {
	MeshBuffer *ret = new MeshBuffer(data_path("spheres.pnct"));
	spheres_for_basic_material = ret->make_vao_for_program(basic_material_program->program);
	return ret;
});

Load< Scene > spheres_scene_multipass(LoadTagLate, []() -> Scene const * {
	return new Scene(data_path("spheres.scene"), [](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = spheres_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable::Pipeline &pipeline = scene.drawables.back().pipeline;
		pipeline = basic_material_program_pipeline;
		pipeline.vao = spheres_for_basic_material;
		pipeline.type = mesh.type;
		pipeline.start = mesh.start;
		pipeline.count = mesh.count;

		float roughness = 1.0f;
		if (transform->name.substr(0, 9) == "Icosphere") {
			roughness = (transform->position.y + 10.0f) / 18.0f;
		}
		pipeline.set_uniforms = [roughness](){
			glUniform1f(basic_material_program->ROUGHNESS_float, roughness);
		};
		
	});
});


DemoLightingMultipassMode::DemoLightingMultipassMode() {

	//Set up camera-only scene:
	{ //create a single camera:
		camera_scene.transforms.emplace_back();
		camera_scene.cameras.emplace_back(&camera_scene.transforms.back());
		scene_camera = &camera_scene.cameras.back();
		scene_camera->fovy = 60.0f / 180.0f * 3.1415926f;
		scene_camera->near = 0.01f;
		//scene_camera->transform and scene_camera->aspect will be set in draw()
	}
}

DemoLightingMultipassMode::~DemoLightingMultipassMode() {
}

bool DemoLightingMultipassMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	//----- leave to menu -----
	if (evt.type == SDL_KEYDOWN && evt.key.keysym.sym == SDLK_ESCAPE) {
		Mode::set_current(demo_menu);
		return true;
	}
	//----- trackball-style camera controls -----
	if (evt.type == SDL_MOUSEBUTTONDOWN) {
		if (evt.button.button == SDL_BUTTON_LEFT) {
			//when camera is upside-down at rotation start, azimuth rotation should be reversed:
			// (this ends up feeling more intuitive)
			camera.flip_x = (std::abs(camera.elevation) > 0.5f * 3.1415926f);
			return true;
		}
	}
	if (evt.type == SDL_MOUSEMOTION) {
		if (evt.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			//figure out the motion (as a fraction of a normalized [-a,a]x[-1,1] window):
			glm::vec2 delta;
			delta.x = evt.motion.xrel / float(window_size.x) * 2.0f;
			delta.x *= float(window_size.y) / float(window_size.x);
			delta.y = evt.motion.yrel / float(window_size.y) * -2.0f;

			if (SDL_GetModState() & KMOD_SHIFT) {
				//shift: pan

				glm::mat3 frame = glm::mat3_cast(scene_camera->transform->rotation);
				camera.target -= frame[0] * (delta.x * camera.radius) + frame[1] * (delta.y * camera.radius);
			} else {
				//no shift: tumble

				camera.azimuth -= 3.0f * delta.x * (camera.flip_x ? -1.0f : 1.0f);
				camera.elevation -= 3.0f * delta.y;

				camera.azimuth /= 2.0f * 3.1415926f;
				camera.azimuth -= std::round(camera.azimuth);
				camera.azimuth *= 2.0f * 3.1415926f;

				camera.elevation /= 2.0f * 3.1415926f;
				camera.elevation -= std::round(camera.elevation);
				camera.elevation *= 2.0f * 3.1415926f;

				//std::cout << camera.azimuth / 3.1415926f * 180.0f << " / " << camera.elevation / 3.1415926f * 180.0f << std::endl;
			}
			return true;
		}
	}
	//mouse wheel: dolly
	if (evt.type == SDL_MOUSEWHEEL) {
		camera.radius *= std::pow(0.5f, 0.1f * evt.wheel.y);
		if (camera.radius < 1e-1f) camera.radius = 1e-1f;
		if (camera.radius > 1e6f) camera.radius = 1e6f;
		return true;
	}
	
	return false;
}

void DemoLightingMultipassMode::draw(glm::uvec2 const &drawable_size) {
	//--- use camera structure to set up scene camera ---

	scene_camera->transform->rotation =
		glm::angleAxis(camera.azimuth, glm::vec3(0.0f, 0.0f, 1.0f))
		* glm::angleAxis(0.5f * 3.1415926f + -camera.elevation, glm::vec3(1.0f, 0.0f, 0.0f))
	;
	scene_camera->transform->position = camera.target + camera.radius * (scene_camera->transform->rotation * glm::vec3(0.0f, 0.0f, 1.0f));
	scene_camera->transform->scale = glm::vec3(1.0f);
	scene_camera->aspect = float(drawable_size.x) / float(drawable_size.y);


	//--- actual drawing ---
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glm::vec3 eye = scene_camera->transform->make_local_to_world()[3];
	glm::mat4 world_to_clip = scene_camera->make_projection() * scene_camera->transform->make_world_to_local();

	for (auto const &light : spheres_scene_multipass->lights) {
		glm::mat4 light_to_world = light.transform->make_local_to_world();
		//set up lighting information for this light:
		glUseProgram(basic_material_program->program);
		glUniform3fv(basic_material_program->EYE_vec3, 1, glm::value_ptr(eye));
		glUniform3fv(basic_material_program->LIGHT_LOCATION_vec3, 1, glm::value_ptr(glm::vec3(light_to_world[3])));
		glUniform3fv(basic_material_program->LIGHT_DIRECTION_vec3, 1, glm::value_ptr(glm::vec3(-light_to_world[2])));
		glUniform3fv(basic_material_program->LIGHT_ENERGY_vec3, 1, glm::value_ptr(light.energy));
		if (light.type == Scene::Light::Point) {
			glUniform1i(basic_material_program->LIGHT_TYPE_int, 0);
			glUniform1f(basic_material_program->LIGHT_CUTOFF_float, 1.0f);
		} else if (light.type == Scene::Light::Hemisphere) {
			glUniform1i(basic_material_program->LIGHT_TYPE_int, 1);
			glUniform1f(basic_material_program->LIGHT_CUTOFF_float, 1.0f);
		} else if (light.type == Scene::Light::Spot) {
			glUniform1i(basic_material_program->LIGHT_TYPE_int, 2);
			glUniform1f(basic_material_program->LIGHT_CUTOFF_float, std::cos(0.5f * light.spot_fov));
		} else if (light.type == Scene::Light::Directional) {
			glUniform1i(basic_material_program->LIGHT_TYPE_int, 3);
			glUniform1f(basic_material_program->LIGHT_CUTOFF_float, 1.0f);
		}
		spheres_scene_multipass->draw(world_to_clip);

		glEnable(GL_BLEND);
		glBlendFunc(GL_ONE, GL_ONE);
		glBlendEquation(GL_FUNC_ADD);
	}
}
