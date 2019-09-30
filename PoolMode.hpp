#pragma once

#include "Mode.hpp"
#include "PoolState.hpp"
#include "Scene.hpp"

#include <memory>

struct PoolMode : Mode {
	PoolMode(std::string const &player_name);
	virtual ~PoolMode();

	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//Current game state:
	PoolState state;

	//Current control signals:
	PoolState::DozerControls controls;

	//Current scene (used for display of game state):
	Scene scene;
	Scene::Camera *camera = nullptr;
	std::vector< Scene::Drawable * > balls;
	std::vector< Scene::Drawable * > dozers;
};
