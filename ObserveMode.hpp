#pragma once

#include "Mode.hpp"
#include "Scene.hpp"

struct ObserveMode : Mode {
	ObserveMode();
	virtual ~ObserveMode();

	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	Scene::Camera const *current_camera = nullptr;
};
