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

	//Can decompose a ray vs cylinder-wall test into a ray-vs-sphere test
	// on a shorter time range:

	//Notably, want that point on ray lies between cylinder end-caps.
	// in other words, 0 < dot(ray_start + ray_direction * t, along) < limit
	glm::vec3 along = cylinder_b - cylinder_a;
	float limit = glm::dot(along, along);
	if (limit == 0.0f) return false;

	//[a0,a1] will be the time range to check:
	float a0 = 0.0f;
	float a1 = 1.0f;
	{ //determine time range [a0,a1] in which closest point to ray is between ends of cylinder:
		float dot_start = glm::dot(ray_start - cylinder_a, along);
		float dot_end = glm::dot(ray_start + ray_direction - cylinder_a, along);

		if (dot_start < 0.0f) {
			if (dot_end <= dot_start) return false;
			a0 = (0.0f - dot_start) / (dot_end - dot_start);
		}
		if (dot_start > limit) {
			if (dot_end >= dot_start) return false;
			a0 = (limit - dot_start) / (dot_end - dot_start);
		}

		if (dot_end < 0.0f) {
			if (dot_start <= dot_end) return false;
			a1 = (0.0f - dot_start) / (dot_end - dot_start);
		}
		if (dot_end > limit) {
			if (dot_start >= dot_end) return false;
			a1 = (limit - dot_start) / (dot_end - dot_start);
		}

	}

	//reduce range by previous collision time:
	if (collision_t) a1 = std::min(a1, *collision_t);

	//if range is empty, nothing to do:
	if (a0 >= a1) return false;

	//closest point on cylinder_a to start of ray:
	glm::vec3 close_start = cylinder_a + glm::dot(ray_start - cylinder_a, along) / glm::dot(along, along) * along;

	//change in closest point over time:
	glm::vec3 close_direction = glm::dot(ray_direction, along) / glm::dot(along, along) * along;

	//construct a ray vs (static) sphere test by viewing the problem relative to the close_start/close_direction sphere and over the [a0,a1] interval:
	float t = a1 - a0;
	if (collide_ray_vs_sphere(
		(ray_start - close_start) + a0 * (ray_direction - close_direction), ray_direction - close_direction,
		glm::vec3(0.0f), cylinder_radius,
		&t, nullptr, nullptr)) {
		t += a0;
		if (collision_t) *collision_t = t;
		if (collision_at) *collision_at = ray_start + t * ray_direction;
		if (collision_out) *collision_out = careful_normalize(ray_start - close_start + t * (ray_direction - close_direction));
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

	{ //check interior of triangle:
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
		glm::vec3 out;
		if (collide_ray_vs_cylinder(sphere_from, (sphere_to - sphere_from),
			triangle_a, triangle_b, sphere_radius,
			collision_t, collision_at, &out)) {
			if (collision_at) *collision_at -= out * sphere_radius;
			if (collision_out) *collision_out = out;
			collided = true;
		}

		if (collide_ray_vs_cylinder(sphere_from, (sphere_to - sphere_from),
			triangle_b, triangle_c, sphere_radius,
			collision_t, collision_at, &out)) {
			if (collision_at) *collision_at -= out * sphere_radius;
			if (collision_out) *collision_out = out;
			collided = true;
		}

		if (collide_ray_vs_cylinder(sphere_from, (sphere_to - sphere_from),
			triangle_c, triangle_a, sphere_radius,
			collision_t, collision_at, &out)) {
			if (collision_at) *collision_at -= out * sphere_radius;
			if (collision_out) *collision_out = out;
			collided = true;
		}
	}

	{ //vertices:
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
