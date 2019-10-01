#pragma once

/*
 * PoolLevel represents the state of a game of pool-dozer.
 *
 */

#include "Scene.hpp"
#include "Load.hpp"

#include <glm/gtx/quaternion.hpp>

#include <vector>
#include <string>
#include <map>

struct PoolLevel;

extern Load< std::list< PoolLevel > > pool_levels;

struct PoolLevel : Scene {
	//empty level:
	PoolLevel() = default;

	//set initial state by reading from a scene:
	PoolLevel(std::string const &scene_file);

	//set initial state by copying from another level:
	PoolLevel(PoolLevel const &);
	PoolLevel &operator=(PoolLevel const &);

	glm::vec2 level_min = glm::vec2(-3.0f, -2.0f);
	glm::vec2 level_max = glm::vec2( 3.0f,  2.0f);
	std::vector< glm::vec2 > goals;

	enum Team : char {
		TeamNone = '\0',
		TeamSolid = 's',
		TeamDiamond = 'd'
	};


	struct Ball {
		Scene::Transform *transform = nullptr;
		float scored = 0.0f; //score amount (used to scale down and remove from physics)
		Team last_to_touch = TeamNone;

		uint32_t index = -1U; //1..15
		bool is_solid() const { return index < 8; }
		bool is_diamond() const { return index > 8; }
		bool is_eight() const { return index == 8; }
	};
	std::list< Ball > balls;

	struct Dozer {
		Scene::Transform *transform = nullptr;
		std::string name = "player";
		float left_tread = 0.0f;
		float right_tread = 0.0f;
		Team team = TeamSolid;

		struct Controls {
			bool left_forward = false;
			bool left_backward = false;
			bool right_forward = false;
			bool right_backward = false;
		} controls;
	};
	std::list< Dozer > dozers;

	//add/remove dozers from the list (creating/removing transforms+drawables):
	Dozer *add_dozer(std::string const &name, Team const &team);
	void remove_dozer(Dozer *dozer);

	//helper to spawn a dozer at a random unoccupied location:
	Dozer *spawn_dozer(std::string const &name);

	//update game state:
	void update(float elapsed);
};
