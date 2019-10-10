#pragma once

#include "Mode.hpp"
#include "Scene.hpp"
#include "Mesh.hpp"

struct DemoLightingMultipassMode : Mode {
	DemoLightingMultipassMode();
	virtual ~DemoLightingMultipassMode();

	virtual bool handle_event(SDL_Event const &, glm::uvec2 const &window_size) override;
	virtual void draw(glm::uvec2 const &drawable_size) override;

	void run_timing();

	//z-up trackball-style camera controls:
	struct {
		float radius = 20.0f;
		float azimuth = 0.5f * 3.1415926f; //angle ccw of -y axis, in radians, [-pi,pi]
		float elevation = 0.2f * 3.1415926f; //angle above ground, in radians, [-pi,pi]
		glm::vec3 target = glm::vec3(0.0f);
		bool flip_x = false; //flip x inputs when moving? (used to handle situations where camera is upside-down)
	} camera;

	//mode uses a secondary Scene to hold a camera:
	Scene camera_scene;
	Scene::Camera *scene_camera = nullptr;
};
