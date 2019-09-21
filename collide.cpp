#include "collide.hpp"

#include <initializer_list>

//helper: normalize but don't return NaN:
glm::vec3 careful_normalize(glm::vec3 const &in) {
	glm::vec3 out = glm::normalize(in);
	//if 'out' ended up as NaN (e.g., because in was a zero vector), reset it:
	if (!(out.x == out.x)) out = glm::vec3(1.0f, 0.0f, 0.0f);
	return out;
}

bool collide_ray_vs_sphere(
	glm::vec3 const &ray_start, glm::vec3 const &ray_direction,
	glm::vec3 const &sphere_center, float sphere_radius,
	float *collision_t, glm::vec3 *collision_at, glm::vec3 *collision_normal) {

	//[when] is? (ray_start + t * ray_direction - sphere_center)^2 <= sphere_center^2

	//Solve a quadratic equation to find out:
	float a = glm::dot(ray_direction, ray_direction);
	float b = 2.0f * glm::dot(ray_start - sphere_center, ray_direction);
	float c = -sphere_radius*sphere_radius;

	//part of the quadratic formula under the radical:
	float d = b*b - 4.0f * a * c;

	if (d < 0.0f) return false;

	d = std::sqrt(d);

	//intersects between t0 and t1:
	float t0 = (-b - d) / (2.0f * a);
	float t1 = (-b + d) / (2.0f * a);

	if (t1 < 0.0f || t0 > 1.0f) return false;
	if (collision_t && t0 >= *collision_t) return false;

	if (t0 <= 0.0f) {
		//collides (or was already colliding) at start:
		if (collision_t) *collision_t = 0.0f;
		if (collision_at) *collision_at = ray_start;
		if (collision_normal) *collision_normal = careful_normalize(ray_start - sphere_center);
		return true;
	} else {
		//collides somewhat after start:
		if (collision_t) *collision_t = t0;
		if (collision_at) *collision_at = ray_start + t0 * ray_direction;
		if (collision_normal) *collision_normal = careful_normalize((ray_start + t0 * ray_direction) - sphere_center);
		return true;
	}
}

bool collide_swept_sphere_vs_point(
	glm::vec3 const &sphere_from, glm::vec3 const &sphere_to, float sphere_radius,
	glm::vec3 const &point,
	float *collision_t, glm::vec3 *collision_at, glm::vec3 *collision_out) {

	//this is the same as ray vs sphere, with a slight re-arrangement of variables:
	return collide_ray_vs_sphere(
		sphere_from, sphere_to - sphere_from,
		point, sphere_radius,
		collision_t, collision_at, collision_out);
}

//Note: doesn't handle endpoints!
bool collide_swept_sphere_vs_line_segment(
	glm::vec3 const &sphere_from, glm::vec3 const &sphere_to, float sphere_radius,
	glm::vec3 const &line_a, glm::vec3 const &line_b,
	float *collision_t, glm::vec3 *collision_at, glm::vec3 *collision_out) {

	//TODO
	return false;
}


bool collide_swept_sphere_vs_box(
	glm::vec3 const &sphere_from, glm::vec3 const &sphere_to, float sphere_radius,
	glm::vec3 const &box_center, glm::vec3 const &box_radius,
	float *collision_t, glm::vec3 *collision_at, glm::vec3 *collision_out
) {
	auto dis = [&box_center,&box_radius](glm::vec3 const &pt) {
		glm::vec3 close = glm::max(box_center-box_radius, glm::min(box_center + box_radius, pt));
		return glm::length(pt - close);
	};
	auto out = [&box_center,&box_radius](glm::vec3 const &pt) {
		glm::vec3 close = glm::max(box_center-box_radius, glm::min(box_center + box_radius, pt));
		return glm::normalize(pt - close);
	};


	float t0 = 0.0f;
	float t1 = 1.0f;
	if (collision_t) t1 = std::min(t1, *collision_t);

	if (t1 <= t0) return false;

	float d0 = dis(sphere_from);

	if (d0 < sphere_radius) {
		glm::vec3 o0 = out(sphere_from);
		if (glm::dot(sphere_to - sphere_from, o0) < 0.0f) {
			if (collision_t) *collision_t = t0;
			if (collision_out) *collision_out = o0;
			if (collision_at) *collision_at = sphere_from - o0 * sphere_radius;
			return true;
		}
	}

	return false;

#if 0
	{ //early-out box vs box check:
		glm::vec3 sphere_min = glm::min(sphere_from, sphere_to) - glm::vec3(sphere_radius);
		glm::vec3 sphere_max = glm::max(sphere_from, sphere_to) - glm::vec3(sphere_radius);
		glm::vec3 box_min = box_center - box_radius;
		glm::vec3 box_max = box_center + box_radius;
		if (sphere_max.x < box_min.x) return false;
		if (sphere_max.y < box_min.y) return false;
		if (sphere_max.z < box_min.z) return false;
		if (sphere_min.x > box_max.x) return false;
		if (sphere_min.y > box_max.y) return false;
		if (sphere_min.z > box_max.z) return false;
	}

	float t = 2.0f;
	bool collided = false;

	if (collision_t) t = std::min(t, *collision_t);

	for (auto const &corner : {
		glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f, -1.0f, -1.0f),
		glm::vec3(-1.0f, -1.0f, -1.0f),
	}) {
		if (collide_swept_sphere_vs_point(sphere_from, sphere_to, sphere_radius,
			box_center + corner * box_radius,
			&t, collision_at, collision_out)) {
			collided = true;
		}
	}

	//TODO: edges, sides.

	return collided;
#endif
}


bool collide_swept_sphere_vs_triangle(
	glm::vec3 const &sphere_from, glm::vec3 const &sphere_to, float sphere_radius,
	glm::vec3 const &triangle_a, glm::vec3 const &triangle_b, glm::vec3 const &triangle_c,
	float *collision_t, glm::vec3 *collision_at, glm::vec3 *collision_out
) {
	//TODO

	return false;
}
