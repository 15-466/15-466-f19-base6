#pragma once

#include "DemoLightingMultipassMode.hpp"

struct DemoLightingDeferredMode : DemoLightingMultipassMode {
	DemoLightingDeferredMode();
	virtual ~DemoLightingDeferredMode();

	enum {
		ShowOutput,
		ShowPosition,
		ShowNormalRoughness,
		ShowAlbedo,
	} show = ShowOutput;

	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;
};
