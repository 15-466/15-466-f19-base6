#pragma once

#include "DemoLightingMultipassMode.hpp"

struct DemoLightingForwardMode : DemoLightingMultipassMode {
	DemoLightingForwardMode();
	virtual ~DemoLightingForwardMode();

	virtual void draw(glm::uvec2 const &drawable_size) override;
};
