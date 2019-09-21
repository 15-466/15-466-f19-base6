#pragma once

#include <glm/glm.hpp>

//Collision functions:

//Check if two AABBs overlap:
// (useful for early-out code)
bool collide_AABB_vs_AABB(
	glm::vec3 const &a_min, glm::vec3 const &a_max,
	glm::vec3 const &b_min, glm::vec3 const &b_max
);

//Check a swept sphere vs a single triangle:
// returns 'true' on collision
bool collide_swept_sphere_vs_triangle(
	//swept sphere:
	glm::vec3 const &sphere_from,
	glm::vec3 const &sphere_to,
	float sphere_radius,
	//triangle:
	glm::vec3 const &triangle_a,
	glm::vec3 const &triangle_b,
	glm::vec3 const &triangle_c,
	//output:
	float *collision_t = nullptr, //[optional,in+out] first time where sphere touches triangle
	glm::vec3 *collision_at = nullptr, //[optional,out] point where sphere touches triangle
	glm::vec3 *collision_out = nullptr //[optional,out] direction to move sphere to get away from triangle as quickly as possible (basically, the outward normal)
);
