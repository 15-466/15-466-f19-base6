#include "DemoLightingForwardMode.hpp"
#include "DrawLines.hpp"
#include "Load.hpp"
#include "data_path.hpp"
#include "BasicMaterialForwardProgram.hpp"
#include "gl_errors.hpp"

#include <glm/gtc/type_ptr.hpp>

#include <iostream>

GLuint spheres_for_basic_material_forward = -1U;
extern Load< MeshBuffer > spheres_meshes;

Load< Scene > spheres_scene_forward(LoadTagLate, []() -> Scene const * {
	spheres_for_basic_material_forward = spheres_meshes->make_vao_for_program(basic_material_forward_program->program);

	return new Scene(data_path("spheres.scene"), [](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const &mesh = spheres_meshes->lookup(mesh_name);

		scene.drawables.emplace_back(transform);
		Scene::Drawable::Pipeline &pipeline = scene.drawables.back().pipeline;
		pipeline = basic_material_forward_program_pipeline;
		pipeline.vao = spheres_for_basic_material_forward;
		pipeline.type = mesh.type;
		pipeline.start = mesh.start;
		pipeline.count = mesh.count;

		float roughness = 1.0f;
		if (transform->name.substr(0, 9) == "Icosphere") {
			roughness = (transform->position.y + 10.0f) / 18.0f;
		}
		pipeline.set_uniforms = [roughness](){
			glUniform1f(basic_material_forward_program->ROUGHNESS_float, roughness);
		};
		
	});
});


DemoLightingForwardMode::DemoLightingForwardMode() {
}

DemoLightingForwardMode::~DemoLightingForwardMode() {
}

void DemoLightingForwardMode::draw(glm::uvec2 const &drawable_size) {
	//--- use camera structure to set up scene camera ---

	scene_camera->transform->rotation =
		glm::angleAxis(camera.azimuth, glm::vec3(0.0f, 0.0f, 1.0f))
		* glm::angleAxis(0.5f * 3.1415926f + -camera.elevation, glm::vec3(1.0f, 0.0f, 0.0f))
	;
	scene_camera->transform->position = camera.target + camera.radius * (scene_camera->transform->rotation * glm::vec3(0.0f, 0.0f, 1.0f));
	scene_camera->transform->scale = glm::vec3(1.0f);
	scene_camera->aspect = float(drawable_size.x) / float(drawable_size.y);


	glm::vec3 eye = scene_camera->transform->make_local_to_world()[3];
	glm::mat4 world_to_clip = scene_camera->make_projection() * scene_camera->transform->make_world_to_local();

	//compute light uniforms:
	uint32_t lights = spheres_scene_forward->lights.size();
	//clamp lights to maximum lights allowed by shader:
	lights = std::min< uint32_t >(lights, BasicMaterialForwardProgram::MaxLights);

	std::vector< int32_t > light_type; light_type.reserve(lights);
	std::vector< glm::vec3 > light_location; light_location.reserve(lights);
	std::vector< glm::vec3 > light_direction; light_direction.reserve(lights);
	std::vector< glm::vec3 > light_energy; light_energy.reserve(lights);
	std::vector< float > light_cutoff; light_cutoff.reserve(lights);

	for (auto const &light : spheres_scene_forward->lights) {
		glm::mat4 light_to_world = light.transform->make_local_to_world();
		//set up lighting information for this light:
		light_location.emplace_back(glm::vec3(light_to_world[3]));
		light_direction.emplace_back(glm::vec3(-light_to_world[2]));
		light_energy.emplace_back(light.energy);

		if (light.type == Scene::Light::Point) {
			light_type.emplace_back(0);
			light_cutoff.emplace_back(1.0f);
		} else if (light.type == Scene::Light::Hemisphere) {
			light_type.emplace_back(1);
			light_cutoff.emplace_back(1.0f);
		} else if (light.type == Scene::Light::Spot) {
			light_type.emplace_back(2);
			light_cutoff.emplace_back(std::cos(0.5f * light.spot_fov));
		} else if (light.type == Scene::Light::Directional) {
			light_type.emplace_back(3);
			light_cutoff.emplace_back(1.0f);
		}

		//skip remaining lights if maximum light count reached:
		if (light_type.size() == lights) break;
	}

	GL_ERRORS();
	
	//--- actual drawing ---
	glClearColor(0.2f, 0.2f, 0.2f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	//upload light uniforms:
	glUseProgram(basic_material_forward_program->program);

	glUniform3fv(basic_material_forward_program->EYE_vec3, 1, glm::value_ptr(eye));

	glUniform1ui(basic_material_forward_program->LIGHTS_uint, lights);

	glUniform1iv(basic_material_forward_program->LIGHT_TYPE_int_array, lights, light_type.data());
	glUniform3fv(basic_material_forward_program->LIGHT_LOCATION_vec3_array, lights, glm::value_ptr(light_location[0]));
	glUniform3fv(basic_material_forward_program->LIGHT_DIRECTION_vec3_array, lights, glm::value_ptr(light_direction[0]));
	glUniform3fv(basic_material_forward_program->LIGHT_ENERGY_vec3_array, lights, glm::value_ptr(light_energy[0]));
	glUniform1fv(basic_material_forward_program->LIGHT_CUTOFF_float_array, lights, light_cutoff.data());

	GL_ERRORS();

	//draw scene:
	spheres_scene_forward->draw(world_to_clip);
}
