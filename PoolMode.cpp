#include "PoolMode.hpp"
#include "DrawLines.hpp"
#include "LitColorTextureProgram.hpp"
#include "Mesh.hpp"
#include "Sprite.hpp"
#include "DrawSprites.hpp"
#include "data_path.hpp"
#include "Sound.hpp"
#include "collide.hpp"
#include "gl_errors.hpp"

//for glm::pow(quaternion, float):
#include <glm/gtx/quaternion.hpp>

#include <algorithm>
#include <iostream>

Load< SpriteAtlas > trade_font_atlas(LoadTagDefault, []() -> SpriteAtlas const * {
	return new SpriteAtlas(data_path("trade-font"));
});

GLuint pool_meshes_for_lit_color_texture_program = 0;

//Load the meshes:
Load< MeshBuffer > pool_meshes(LoadTagDefault, []() -> MeshBuffer * {
	MeshBuffer *ret = new MeshBuffer(data_path("pool.pnct"));
	return ret;
});

//Load the pool table scene:
Load< Scene > pool_scene(LoadTagDefault, []() -> Scene * {
	return new Scene(data_path("pool.scene"), [](Scene &scene, Scene::Transform *transform, std::string const &mesh_name){
		Mesh const *mesh = &pool_meshes->lookup(mesh_name);
	
		scene.drawables.emplace_back(transform);
		Scene::Drawable::Pipeline &pipeline = scene.drawables.back().pipeline;
		
		//set up drawable to draw mesh from buffer:
		pipeline = lit_color_texture_program_pipeline;
		pipeline.vao = pool_meshes_for_lit_color_texture_program;
		pipeline.type = mesh->type;
		pipeline.start = mesh->start;
		pipeline.count = mesh->count;
	});
});


PoolMode::PoolMode(std::string const &player_name) : scene(*pool_scene) {
	
}

PoolMode::~PoolMode() {
}

bool PoolMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	if (evt.type == SDL_KEYDOWN || evt.type == SDL_KEYUP) {
		if (evt.key.keysym.scancode == SDL_SCANCODE_W) {
			controls.left_forward = (evt.type == SDL_KEYDOWN);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_S) {
			controls.left_backward = (evt.type == SDL_KEYDOWN);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_UP) {
			controls.right_forward = (evt.type == SDL_KEYDOWN);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_DOWN) {
			controls.right_backward = (evt.type == SDL_KEYDOWN);
			return true;
		}
	}

	return false;
}

void PoolMode::update(float elapsed) {
}

void PoolMode::draw(glm::uvec2 const &drawable_size) {
	//Sync scene with state:
	{ //balls:
		/*
		while (balls.size() < state.balls.size()) {
			scene.transforms.emplace_back();
			scene.drawables.emplace_back(&scene.transforms.back());


			balls.
		}
		*/
	}
	{ //dozers:
		
	}

	//--- actual drawing ---
	glClearColor(0.45f, 0.45f, 0.50f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	camera->aspect = drawable_size.x / float(drawable_size.y);
	scene.draw(*camera);

	{ //help text overlay:
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		DrawSprites draw(*trade_font_atlas, glm::vec2(0,0), glm::vec2(320, 200), drawable_size, DrawSprites::AlignPixelPerfect);

		{
			std::string help_text = "w/s + up/down:move";
			glm::vec2 min, max;
			draw.get_text_extents(help_text, glm::vec2(0.0f, 0.0f), 1.0f, &min, &max);
			float x = std::round(160.0f - (0.5f * (max.x + min.x)));
			draw.draw_text(help_text, glm::vec2(x, 1.0f), 1.0f, glm::u8vec4(0x00,0x00,0x00,0xff));
			draw.draw_text(help_text, glm::vec2(x, 2.0f), 1.0f, glm::u8vec4(0xff,0xff,0xff,0xff));
		}

	}

	/*if (DEBUG_draw_lines) { //DEBUG drawing:
		//adjust world-to-clip matrix to current camera:
		DEBUG_draw_lines->world_to_clip = level.camera->make_projection() * level.camera->transform->make_world_to_local();
		//delete object (draws in destructor):
		DEBUG_draw_lines.reset();
	}*/


	GL_ERRORS();
}
