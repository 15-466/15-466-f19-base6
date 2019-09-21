#pragma once

/*
 * A RollLevel is a scene augmented with some additional information which
 *   is useful when playing Sphere Roll.
 */

#include "Scene.hpp"
#include "Mesh.hpp"
#include "Load.hpp"

struct RollLevel;

//List of all levels:
extern Load< std::list< RollLevel > > roll_levels;

struct RollLevel : Scene {
	//Build from a scene:
	//  note: will throw on loading failure, or if certain critical objects don't appear
	RollLevel(std::string const &scene_file);

	//Copy constructor:
	//  used to copy a pristine, just-loaded level to a level that is being played
	//    (and thus might be changed)
	//  -- needs to be careful to fixup pointers.
	RollLevel(RollLevel const &);
	// copy constructor actually just uses this = operator:
	RollLevel &operator=(RollLevel const &);

	
	//Solid parts of level are tracked as MeshColliders:
	struct MeshCollider {
		MeshCollider(Scene::Transform *transform_, Mesh const &mesh_, MeshBuffer const &buffer_) : transform(transform_), mesh(&mesh_), buffer(&buffer_) { }
		Scene::Transform *transform;
		Mesh const *mesh;
		MeshBuffer const *buffer;
	};

	//Goal objects(s) tracked using this structure:
	struct Goal {
		Goal(Scene::Transform *transform_) : transform(transform_) { };
		Scene::Transform *transform;
		float spin_acc = 0.0f;
	};

	//Sphere being rolled tracked using this structure:
	struct Player {
		Scene::Transform *transform = nullptr;
		glm::vec3 rotational_velocity = glm::vec3(0.0f, 0.0f, 0.0f);
		glm::vec3 velocity = glm::vec3(0.0f, 0.0f, 0.0f);

		float view_azimuth = 0.0f;
		float view_elevation = 45.0f / 180.0f * 3.1415926f;
	};

	//Additional information for things in the level:
	std::vector< MeshCollider > mesh_colliders;
	std::vector< Goal > goals;
	Player player;

	Scene::Camera *camera = nullptr;
};

