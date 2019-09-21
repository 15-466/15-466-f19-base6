#pragma once

#include <glm/glm.hpp>

//Collision functions:

//Check a swept sphere vs an axis-aligned box:
// returns 'true' on collision
// note: if collision_t is set, will not report any collision after min(collision_t, 1.0f)
bool collide_swept_sphere_vs_box(
	//swept sphere:
	glm::vec3 const &sphere_from,
	glm::vec3 const &sphere_to,
	float sphere_radius,
	//box:
	glm::vec3 const &box_center,
	glm::vec3 const &box_radius,
	//output:
	float *collision_t = nullptr, //[optional,in+out] first time where sphere touches box
	glm::vec3 *collision_at = nullptr, //[optional,out] point where sphere touches box
	glm::vec3 *collision_out = nullptr //[optional,out] direction to move sphere to get away from box as quickly as possible (basically, the outward normal)
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
