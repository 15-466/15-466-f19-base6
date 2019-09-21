#pragma once

#include "Mode.hpp"
#include "RollLevel.hpp"
#include "DrawLines.hpp"

#include <memory>

struct RollMode : Mode {
	RollMode(RollLevel const &level);
	virtual ~RollMode();

	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void update(float elapsed) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	//The (starting shape of the) level:
	RollLevel const &start;

	//The (active, being-played) level:
	void restart();
	RollLevel level;
	bool won = false;

	//Current control signals:
	struct {
		bool forward = false;
		bool backward = false;
		bool left = false;
		bool right = false;
	} controls;

	//fly around for collsion debug:
	bool DEBUG_fly = false;
	bool DEBUG_show_geometry = false;
	bool DEBUG_show_collision = false;

	//some debug drawing done during update:
	std::unique_ptr< DrawLines > DEBUG_draw_lines;
};
