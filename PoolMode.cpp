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


PoolMode::PoolMode(PoolLevel const &start_, std::string const &port) : start(start_) {
	if (port != "") {
		server.reset(new Server(port));
	}
	restart();
}

PoolMode::~PoolMode() {
}

void PoolMode::restart() {
	level = start;

	//spawn remote players:
	if (server) {
		for (auto &ci : connection_infos) {
			ci.second.dozer = level.spawn_dozer(ci.second.name);
		}
	}

	//spawn a local player:
	dozer = level.spawn_dozer("Local");

	if (level.cameras.empty()) {
		throw std::runtime_error("Level is missing a camera.");
	}
	camera = &*level.cameras.begin();
}

bool PoolMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	if (dozer) {
		if (evt.type == SDL_KEYDOWN || evt.type == SDL_KEYUP) {
			if (evt.key.keysym.scancode == SDL_SCANCODE_W) {
				dozer->controls.left_forward = (evt.type == SDL_KEYDOWN);
				return true;
			} else if (evt.key.keysym.scancode == SDL_SCANCODE_S) {
				dozer->controls.left_backward = (evt.type == SDL_KEYDOWN);
				return true;
			} else if (evt.key.keysym.scancode == SDL_SCANCODE_UP) {
				dozer->controls.right_forward = (evt.type == SDL_KEYDOWN);
				return true;
			} else if (evt.key.keysym.scancode == SDL_SCANCODE_DOWN) {
				dozer->controls.right_backward = (evt.type == SDL_KEYDOWN);
				return true;
			}
		}
	}

	return false;
}

void PoolMode::update(float elapsed) {
	//read from remote players:
	if (server) {
		auto remove_player = [this](Connection *c) {
			auto f = connection_infos.find(c);
			if (f != connection_infos.end()) {
				level.remove_dozer(f->second.dozer);
				connection_infos.erase(f);
			}
		};
		server->poll([this,&remove_player](Connection *c, Connection::Event evt) {
			if (evt == Connection::OnOpen) {
				auto &info = connection_infos[c];
				info.name = "[" + std::to_string(c->socket) + "]";
				info.dozer = level.spawn_dozer(info.name);
			} else if (evt == Connection::OnClose) {
				remove_player(c);
			} else if (evt == Connection::OnRecv) {
				auto &info = connection_infos[c];
				//update controls:
				while (c->recv_buffer.size() >= 4) {
					uint8_t type = c->recv_buffer[0];
					uint32_t length = uint32_t(c->recv_buffer[1]) << 16 | uint32_t(c->recv_buffer[2]) << 8 | uint32_t(c->recv_buffer[3]);
					if (type == 'C') {
						if (length != 1) {
							//invalid length for a control message:
							remove_player(c);
							c->close();
							return;
						} else if (c->recv_buffer.size() < 4 + length) {
							//need more data
							break;
						} else {
							//got a whole controls message:
							uint8_t controls = c->recv_buffer[5];
							//erase from buffer:
							c->recv_buffer.erase(c->recv_buffer.begin(), c->recv_buffer.begin() + 4 + length);
							//set controls using message:
							info.dozer->controls.left_forward = (controls & 1);
							info.dozer->controls.left_backward = (controls & 2);
							info.dozer->controls.right_forward = (controls & 3);
							info.dozer->controls.right_backward = (controls & 4);
						}
					}
				}
			}
		}, 0.0);
	}
	

	level.update(elapsed);
}

void PoolMode::draw(glm::uvec2 const &drawable_size) {
	//--- actual drawing ---
	glClearColor(0.45f, 0.45f, 0.50f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	camera->aspect = drawable_size.x / float(drawable_size.y);
	level.draw(*camera);

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
