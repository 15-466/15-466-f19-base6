#pragma once

#include "DemoLightingMultipassMode.hpp"

struct DemoLightingDeferredMode : DemoLightingMultipassMode {
	DemoLightingDeferredMode();
	virtual ~DemoLightingDeferredMode();

	virtual void draw(glm::uvec2 const &drawable_size) override;
};
