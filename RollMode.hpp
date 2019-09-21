#pragma once

#include "Mode.hpp"
#include "RollLevel.hpp"

struct RollMode : Mode {
	RollMode(RollLevel const &level);
	virtual ~RollMode();

	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//The (starting shape of the) level:
	RollLevel const &start;

	//The (active, being-played) level:
	RollLevel level;

	//Current control signals:
	struct {
		bool forward = false;
		bool backward = false;
		bool left = false;
		bool right = false;
	} controls;
};
