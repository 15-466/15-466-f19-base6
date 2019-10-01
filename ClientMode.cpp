#include "ClientMode.hpp"
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


ClientMode::ClientMode(std::string const &host, std::string const &port) {
	client.reset(new Client(host, port));
}

ClientMode::~ClientMode() {
}

bool ClientMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
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

void ClientMode::update(float elapsed) {
	//send current controls:
	client->connection.send_buffer.emplace_back('C');
	client->connection.send_buffer.emplace_back(0);
	client->connection.send_buffer.emplace_back(0);
	client->connection.send_buffer.emplace_back(1);
	client->connection.send_buffer.emplace_back(
		  (controls.left_forward  ? 1 : 0)
		| (controls.left_backward ? 2 : 0)
		| (controls.right_forward  ? 4 : 0)
		| (controls.right_backward ? 8 : 0)
	);

	client->poll([this](Connection *, Connection::Event evt){
		//TODO: eventually, read server state
	}, 0.0);
	//if connection was closed,
	if (!client->connection) {
		Mode::set_current(nullptr);
	}
}

void ClientMode::draw(glm::uvec2 const &drawable_size) {
	//--- actual drawing ---
	glClearColor(0.45f, 0.45f, 0.50f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

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
