#include "collide.hpp"

#include <initializer_list>
#include <iostream>


//Check if two AABBs overlap:
// (useful for early-out code)
bool collide_AABB_vs_AABB(
	glm::vec3 const &a_min, glm::vec3 const &a_max,
	glm::vec3 const &b_min, glm::vec3 const &b_max
) {
	return !(
		   (a_max.x < b_min.x)
		|| (a_max.y < b_min.y)
		|| (a_max.z < b_min.z)
		|| (b_max.x < a_min.x)
		|| (b_max.y < a_min.y)
		|| (b_max.z < a_min.z)
	);
}



//helper: normalize but don't return NaN:
glm::vec3 careful_normalize(glm::vec3 const &in) {
	glm::vec3 out = glm::normalize(in);
	//if 'out' ended up as NaN (e.g., because in was a zero vector), reset it:
	if (!(out.x == out.x)) out = glm::vec3(1.0f, 0.0f, 0.0f);
	return out;
}

//helper: ray vs sphere:
bool collide_ray_vs_sphere(
	glm::vec3 const &ray_start, glm::vec3 const &ray_direction,
	glm::vec3 const &sphere_center, float sphere_radius,
	float *collision_t, glm::vec3 *collision_at, glm::vec3 *collision_normal) {

	//if ray is travelling away from sphere, don't intersect:
	if (glm::dot(ray_start - sphere_center, ray_direction) >= 0.0f) return false;

	//when is (ray_start + t * ray_direction - sphere_center)^2 <= sphere_radius^2 ?

	//Solve a quadratic equation to find out:
	float a = glm::dot(ray_direction, ray_direction);
	float b = 2.0f * glm::dot(ray_start - sphere_center, ray_direction);
	float c = glm::dot(ray_start - sphere_center, ray_start - sphere_center) - sphere_radius*sphere_radius;

	//this is the part of the quadratic formula under the radical:
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

//helper: ray vs cylinder segment
// note: doesn't handle endpoints!
bool collide_ray_vs_cylinder(
	glm::vec3 const &ray_start, glm::vec3 const &ray_direction,
	glm::vec3 const &cylinder_a, glm::vec3 const &cylinder_b, float cylinder_radius,
	float *collision_t, glm::vec3 *collision_at, glm::vec3 *collision_out) {

	glm::vec3 along = cylinder_b - cylinder_a;
	if (along == glm::vec3(0.0f)) return false;

	float a0, a1;
	{ //determine time range [a0,a1] in which closest point to ray is between ends of cylinder:
		float dot_start = glm::dot(ray_start - cylinder_a, along);
		float dot_end = glm::dot(ray_start + ray_direction - cylinder_a, along);
		float limit = glm::dot(along, along);

		if (dot_start < dot_end) {
			if (dot_start >= limit) {
				if (dot_end >= limit) return false;
				a0 = (limit - dot_start) / (dot_end - dot_start);
			} else {
				a0 = 0.0f;
			}
			if (dot_end <= 0.0f) {
				if (dot_start <= 0.0f) return false;
				a1 = (0.0f - dot_start) / (dot_end - dot_start);
			} else {
				a1 = 1.0f;
			}
		} else if (dot_start < dot_end) {
			if (dot_start <= 0.0f) {
				if (dot_end <= 0.0f) return false;
				a0 = (0.0f - dot_start) / (dot_end - dot_start);
			} else {
				a0 = 0.0f;
			}
			if (dot_end >= limit) {
				if (dot_start >= limit) return false;
				a1 = (limit - dot_start) / (dot_end - dot_start);
			} else {
				a1 = 1.0f;
			}
		} else { //dot_start == dot_end
			//start and end are both within cylinder, so consider entire time range:
			if (dot_start <= 0.0f || dot_start >= limit) return false;
			a0 = 0.0f;
			a1 = 1.0f;
		}
		if (a0 >= a1) return false;
	}

	//closest point on cylinder_a -> cylinder_b to start of ray:
	glm::vec3 close_start = ray_start + glm::dot(cylinder_a - ray_start, along) / glm::dot(along, along) * along;

	//change in closest point over time:
	glm::vec3 close_direction = glm::dot(ray_direction, along) / glm::dot(along, along) * along;

	//when is (ray_start + t * ray_direction - close_start - t * close_direction)^2 <= cylinder_radius^2 ?

	// can solve a quadratic equation:
	float a = glm::dot(ray_direction - close_direction, ray_direction - close_direction);
	float b = 2.0f * glm::dot(ray_start - close_start, ray_direction - close_direction);
	float c = glm::dot(ray_start - close_start, ray_start - close_start) - cylinder_radius*cylinder_radius;

	//quick check: don't intersect if derivative is non-decreasing at t == 0:
	if (b >= 0.0f) return false;


	//this is the part of the quadratic formula under the radical:
	float d = b*b - 4.0f * a * c;
	if (d < 0.0f) return false;
	d = std::sqrt(d);

	//intersects between t0 and t1:
	float t0 = (-b - d) / (2.0f * a);
	float t1 = (-b + d) / (2.0f * a);


	//DEBUG, shouldn't have nan's creep in here:
	assert(t0 == t0);
	assert(t1 == t1);

	//if that doesn't happen during ray, no collision:
	if (t1 <= 0.0f || t0 >= 1.0f) return false;

	//restrict to the range where closest point is actually between start and end of cylinder:
	t0 = std::max(t0, a0);
	t1 = std::min(t1, a1);

	//if that intersection is empty, no collision:
	if (t0 >= t1) return false;

	//DEBUG:
	std::cout << t0 << " => " << glm::length(ray_start - close_start + t0 * (ray_direction - close_direction)) << " == " << cylinder_radius << std::endl;

	//if existing collision is earlier, no collision:
	if (collision_t && *collision_t <= t0) return false;

	if (t0 <= 0.0f) {
		//collides (or was already colliding) at start:
		if (collision_t) *collision_t = 0.0f;
		if (collision_at) *collision_at = ray_start;
		if (collision_out) *collision_out = careful_normalize(ray_start - close_start);
		return true;
	} else {
		//collides somewhere after start:
		if (collision_t) *collision_t = t0;
		if (collision_at) *collision_at = ray_start + ray_direction * t0;
		if (collision_out) *collision_out = careful_normalize(ray_start - close_start + t0 * (ray_direction - close_direction));
		return true;
	}

	return false;
}

bool collide_swept_sphere_vs_triangle(
	glm::vec3 const &sphere_from, glm::vec3 const &sphere_to, float sphere_radius,
	glm::vec3 const &triangle_a, glm::vec3 const &triangle_b, glm::vec3 const &triangle_c,
	float *collision_t, glm::vec3 *collision_at, glm::vec3 *collision_out
) {

	float t = 2.0f;

	if (collision_t) {
		t = std::min(t, *collision_t);
		if (t <= 0.0f) return false;
	}

	if (false) { //check interior of triangle:
		//vector perpendicular to plane:
		glm::vec3 perp = glm::cross(triangle_b - triangle_a, triangle_c - triangle_a);
		if (perp == glm::vec3(0.0f)) {
			//degenerate triangle, skip plane test
		} else {
			glm::vec3 normal = glm::normalize(perp);
			float dot_from = glm::dot(normal, sphere_from - triangle_a);
			float dot_to = glm::dot(normal, sphere_to - triangle_a);

			//determine time range when sphere overlaps plane containing triangle:

			//start with empty range:
			float t0 = 2.0f;
			float t1 =-1.0f;
			if (dot_from < 0.0f && dot_to > dot_from) {
				//approaching triangle from below
				t0 = (-sphere_radius - dot_from) / (dot_to - dot_from);
				t1 = ( sphere_radius - dot_from) / (dot_to - dot_from);
			} else if (dot_from > 0.0f && dot_to < dot_from) {
				//approaching triangle from above
				t0 = ( sphere_radius - dot_from) / (dot_to - dot_from);
				t1 = (-sphere_radius - dot_from) / (dot_to - dot_from);
			}
			//if range doesn't overlap [0,t], no collision (with plane, vertices, or edges) is possible:
			if (t1 < 0.0f || t0 >= t) {
				return false;
			}

			//check if collision with happens inside triangle:
			float at_t = glm::max(0.0f, t0);
			glm::vec3 at = glm::mix(sphere_from, sphere_to, at_t);

			//close is 'at' projected to the plane of the triangle:
			glm::vec3 close = at + glm::dot(triangle_a - at, normal) * normal;

			//check if 'close' is inside triangle:
			//  METHOD: this is checking whether the triangles (b,c,x), (c,a,x), and (a,b,x) have the same orientation as (a,b,c)
			if ( glm::dot(glm::cross(triangle_b - triangle_a, close - triangle_a), normal) >= 0
			  && glm::dot(glm::cross(triangle_c - triangle_b, close - triangle_b), normal) >= 0
			  && glm::dot(glm::cross(triangle_a - triangle_c, close - triangle_c), normal) >= 0 ) {
				//sphere does intersect with triangle inside the triangle:
				if (collision_t) *collision_t = at_t;
				if (collision_out) *collision_out = careful_normalize(at - close);
				if (collision_at) *collision_at = close;
				return true;
			}
		}
	}

	//Plane check hasn't been able to reject sphere sweep, move on to checking edges and vertices:
	bool collided = false;

	{ //edges
		if (collide_ray_vs_cylinder(sphere_from, (sphere_to - sphere_from),
			triangle_a, triangle_b, sphere_radius,
			collision_t, collision_at, collision_out)) {
			collided = true;
		}

		if (collide_ray_vs_cylinder(sphere_from, (sphere_to - sphere_from),
			triangle_b, triangle_c, sphere_radius,
			collision_t, collision_at, collision_out)) {
			collided = true;
		}

		if (collide_ray_vs_cylinder(sphere_from, (sphere_to - sphere_from),
			triangle_c, triangle_a, sphere_radius,
			collision_t, collision_at, collision_out)) {
			collided = true;
		}
	}

	if (false) { //vertices:
		if (collide_ray_vs_sphere(sphere_from, (sphere_to - sphere_from),
			triangle_a, sphere_radius,
			collision_t, nullptr, collision_out)) {
			if (collision_at) *collision_at = triangle_a;
			collided = true;
		}
		if (collide_ray_vs_sphere(sphere_from, (sphere_to - sphere_from),
			triangle_b, sphere_radius,
			collision_t, nullptr, collision_out)) {
			if (collision_at) *collision_at = triangle_b;
			collided = true;
		}
		if (collide_ray_vs_sphere(sphere_from, (sphere_to - sphere_from),
			triangle_c, sphere_radius,
			collision_t, nullptr, collision_out)) {
			if (collision_at) *collision_at = triangle_c;
			collided = true;
		}
	}

	return collided;
}
