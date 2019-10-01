#pragma once

#include "Mode.hpp"
#include "PoolLevel.hpp"
#include "Scene.hpp"

#include <memory>

struct PoolMode : Mode {
	PoolMode(PoolLevel const &level);
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
	PoolLevel::Dozer *dozer = nullptr;
	Scene::Camera *camera = nullptr;
};
