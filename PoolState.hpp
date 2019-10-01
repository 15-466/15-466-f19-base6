#pragma once

/*
 * PoolState represents the state of a game of pool-dozer.
 *
 */

#include <glm/gtx/quaternion.hpp>

#include <vector>
#include <string>
#include <map>

struct PoolState {
	struct BallState {
		uint32_t index = -1U;
		glm::vec3 position = glm::vec3(0.0f);
		glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
		glm::vec2 velocity = glm::vec2(0.0f);
		float respawn = 0.0f;
		enum : char {
			Solid = 's',
			Diamond = 'd'
		} type = Solid;
	};
	std::vector< BallState > balls;

	struct DozerState {
		std::string name = "player";
		glm::vec2 position = glm::vec2(0.0f);
		float angle = 0.0f;
		float left_tread = 0.0f;
		float right_tread = 0.0f;
		enum : char {
			Solid = 's',
			Diamond = 'd'
		} team = Solid;
	};
	std::vector< DozerState > dozers;

	struct DozerControls {
		bool left_forward = false;
		bool left_backward = false;
		bool right_forward = false;
		bool right_backward = false;
	};

	//update game state, given controls for each dozer:
	void update(std::map< DozerState const *, DozerControls > const &controls, float elapsed);
};
