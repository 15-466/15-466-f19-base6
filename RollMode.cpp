#include "RollMode.hpp"
#include "DrawLines.hpp"
#include "LitColorTextureProgram.hpp"
#include "Mesh.hpp"
#include "Sprite.hpp"
#include "DrawSprites.hpp"
#include "data_path.hpp"
#include "Sound.hpp"
#include "collide.hpp"

#include <iostream>

Load< SpriteAtlas > trade_font_atlas(LoadTagDefault, []() -> SpriteAtlas const * {
	return new SpriteAtlas(data_path("trade-font"));
});

RollMode::RollMode(RollLevel const &level_) : start(level_), level(start) {
}

RollMode::~RollMode() {
}

bool RollMode::handle_event(SDL_Event const &evt, glm::uvec2 const &window_size) {
	if (evt.type == SDL_KEYDOWN || evt.type == SDL_KEYUP) {
		if (evt.key.keysym.scancode == SDL_SCANCODE_A) {
			controls.left = (evt.type == SDL_KEYDOWN);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_D) {
			controls.right = (evt.type == SDL_KEYDOWN);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_W) {
			controls.forward = (evt.type == SDL_KEYDOWN);
			return true;
		} else if (evt.key.keysym.scancode == SDL_SCANCODE_S) {
			controls.backward = (evt.type == SDL_KEYDOWN);
			return true;
		}
	}

	if (evt.type == SDL_MOUSEMOTION) {
		if (evt.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
			//based on trackball camera control from ShowMeshesMode:
			//figure out the motion (as a fraction of a normalized [-a,a]x[-1,1] window):
			glm::vec2 delta;
			delta.x = evt.motion.xrel / float(window_size.x) * 2.0f;
			delta.x *= float(window_size.y) / float(window_size.x);
			delta.y = evt.motion.yrel / float(window_size.y) * -2.0f;

			level.player.view_azimuth -= delta.x;
			level.player.view_elevation -= delta.y;

			level.player.view_azimuth /= 2.0f * 3.1415926f;
			level.player.view_azimuth -= std::round(level.player.view_azimuth);
			level.player.view_azimuth *= 2.0f * 3.1415926f;

			level.player.view_elevation = std::max(-85.0f / 180.0f * 3.1415926f, level.player.view_elevation);
			level.player.view_elevation = std::min( 85.0f / 180.0f * 3.1415926f, level.player.view_elevation);

			return true;
		}
	}
	
	return false;
}

void RollMode::update(float elapsed) {

	{ //player motion:
		//build a shove from controls:

		glm::vec3 shove = glm::vec3(0.0f);
		if (controls.left) shove.x -= 1.0f;
		if (controls.right) shove.x += 1.0f;
		if (controls.forward) shove.y += 1.0f;
		if (controls.backward) shove.y -= 1.0f;

		if (shove != glm::vec3(0.0f)) {
			//make shove relative to camera direction (but ignore up/down tilt):
			shove = glm::normalize(shove);
			float ca = std::cos(level.player.view_azimuth);
			float sa = std::sin(level.player.view_azimuth);
			shove = glm::vec3(ca, sa, 0.0f) * shove.x + glm::vec3(-sa, ca, 0.0f) * shove.y;
		}

		shove *= 10.0f;

		//update player using shove:

		glm::vec3 &position = level.player.transform->position;
		glm::vec3 &velocity = level.player.velocity;

		//decay existing velocity toward shove:
		velocity = glm::mix(shove, velocity, std::pow(0.5f, elapsed / 0.25f));
		
		//collide against level:
		float remain = elapsed;
		for (int32_t iter = 0; iter < 10; ++iter) {
			if (remain == 0.0f) break;

			glm::vec3 from = position;
			glm::vec3 to = position + velocity * remain;

			bool collided = false;
			float t = 2.0f;
			glm::vec3 collision_at = glm::vec3(0.0f);
			glm::vec3 collision_out = glm::vec3(0.0f);
			for (auto const &box : level.box_colliders) {
				glm::mat4x3 box_to_world = box.transform->make_local_to_world();
				glm::vec3 box_min = box_to_world[3]
					- glm::abs(box.radius.x * box_to_world[0])
					- glm::abs(box.radius.y * box_to_world[1])
					- glm::abs(box.radius.z * box_to_world[2]);
				glm::vec3 box_max = box_to_world[3]
					+ glm::abs(box.radius.x * box_to_world[0])
					+ glm::abs(box.radius.y * box_to_world[1])
					+ glm::abs(box.radius.z * box_to_world[2]);

				glm::vec3 box_center = 0.5f * (box_max + box_min);
				glm::vec3 box_radius = 0.5f * (box_max - box_min);

				if (collide_swept_sphere_vs_box(
					from, to, 1.0f,
					box_center, box_radius,
					&t, &collision_at, &collision_out)) {
					collided = true;
				}
			}

			if (!collided) {
				position = to;
				remain = 0.0f;
				break;
			} else {
				position = glm::mix(from, to, t);
				float d = glm::dot(velocity, collision_out);
				if (d < 0.0f) velocity -= 1.5f * d * collision_out;
				remain = (1.0f - t) * remain;
			}
		}

	}

	//goal update:
	for (auto &goal : level.goals) {
		goal.spin_acc += elapsed / 10.0f;
		goal.spin_acc -= std::floor(goal.spin_acc);
		goal.transform->rotation = glm::angleAxis(goal.spin_acc * 2.0f * 3.1415926f, glm::normalize(glm::vec3(1.0f)));
	}

	{ //camera update:
		level.camera->transform->rotation =
			glm::angleAxis( level.player.view_azimuth, glm::vec3(0.0f, 0.0f, 1.0f) )
			* glm::angleAxis(-level.player.view_elevation + 0.5f * 3.1415926f, glm::vec3(1.0f, 0.0f, 0.0f) )
		;
		glm::vec3 in = level.camera->transform->rotation * glm::vec3(0.0f, 0.0f, -1.0f);
		level.camera->transform->position = level.player.transform->position - 10.0f * in;
	}
}

void RollMode::draw(glm::uvec2 const &drawable_size) {
	//--- actual drawing ---
	glClearColor(0.85f, 0.85f, 0.90f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	level.camera->aspect = drawable_size.x / float(drawable_size.y);
	level.draw(*level.camera);

	{ //help text overlay:
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		DrawSprites draw(*trade_font_atlas, glm::vec2(0,0), glm::vec2(320, 200), drawable_size, DrawSprites::AlignPixelPerfect);

		std::string help_text = "--- SWITCH CAMERAS WITH LEFT/RIGHT ---";
		glm::vec2 min, max;
		draw.get_text_extents(help_text, glm::vec2(0.0f, 0.0f), 1.0f, &min, &max);
		float x = std::round(160.0f - (0.5f * (max.x + min.x)));
		draw.draw_text(help_text, glm::vec2(x, 1.0f), 1.0f, glm::u8vec4(0x00,0x00,0x00,0xff));
		draw.draw_text(help_text, glm::vec2(x, 2.0f), 1.0f, glm::u8vec4(0xff,0xff,0xff,0xff));
	}
}
