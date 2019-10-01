#pragma once

#include "Mode.hpp"
#include "PoolLevel.hpp"
#include "Scene.hpp"
#include "Connection.hpp"

#include <memory>

struct PoolMode : Mode {
	PoolMode(PoolLevel const &level, std::string const &server_port = "");
	virtual ~PoolMode();

	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//helper: restart level
	void restart();

	//Game starting state:
	PoolLevel const &start;

	//Current game state:
	PoolLevel level;

	//Access to level:
	Scene::Camera *camera = nullptr;

	//local player:
	PoolLevel::Dozer *dozer = nullptr;

	//remote players:
	std::unique_ptr< Server > server;
	struct PlayerInfo {
		std::string name;
		PoolLevel::Dozer *dozer = nullptr;
	};
	std::unordered_map< Connection const *, PlayerInfo > connection_infos;
};
