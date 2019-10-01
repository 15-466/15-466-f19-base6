#include "PoolLevel.hpp"
#include "Mesh.hpp"
#include "LitColorTextureProgram.hpp"
#include "data_path.hpp"

#include <iostream>

GLuint pool_meshes_for_lit_color_texture_program = 0;

Mesh const *mesh_DozerDiamond = nullptr;
Mesh const *mesh_DozerSolid = nullptr;

Load< MeshBuffer > pool_meshes(LoadTagDefault, []() -> MeshBuffer * {
	MeshBuffer *ret = new MeshBuffer(data_path("pool.pnct"));

	mesh_DozerDiamond = &ret->lookup("Dozer.Diamond");
	mesh_DozerSolid = &ret->lookup("Dozer.Solid");

	//Build vertex array object for the program we're using to shade these meshes:
	pool_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);

	return ret;
});

Load< std::list< PoolLevel > > pool_levels(LoadTagLate, []() -> std::list< PoolLevel > const * {
	std::list< PoolLevel > *list = new std::list< PoolLevel >();
	list->emplace_back(data_path("pool.scene"));
	return list;
});

PoolLevel::PoolLevel(std::string const &scene_file) {
	load(scene_file, [this,&scene_file](Scene &, Transform *transform, std::string const &mesh_name){
		Mesh const *mesh = &pool_meshes->lookup(mesh_name);
	
		drawables.emplace_back(transform);
		Drawable::Pipeline &pipeline = drawables.back().pipeline;
		
		//set up drawable to draw mesh from buffer:
		pipeline = lit_color_texture_program_pipeline;
		pipeline.vao = pool_meshes_for_lit_color_texture_program;
		pipeline.type = mesh->type;
		pipeline.start = mesh->start;
		pipeline.count = mesh->count;


		//associate level info with the drawable:
		if (mesh_name.substr(0,5) == "Ball.") {
			int32_t index = std::stoi(mesh_name.substr(5));
			if (!(index >= 1 && index <= 15)) {
				throw std::runtime_error("Ball '" + mesh_name + "' with invalid index.");
			}
			balls.emplace_back();
			balls.back().transform = transform;
			balls.back().index = index;
		} else if (mesh_name.substr(0,5) == "Goal.") {
			goals.emplace_back(glm::vec2(transform->position));
		}
	});
}

PoolLevel::PoolLevel(PoolLevel const &other) {
	*this = other;
}

PoolLevel &PoolLevel::operator=(PoolLevel const &other) {
	std::unordered_map< Transform const *, Transform * > transform_to_transform;
	set(other, &transform_to_transform);
	level_min = other.level_min;
	level_max = other.level_max;
	goals = other.goals;
	balls = other.balls;
	for (auto &ball : balls) {
		ball.transform = transform_to_transform.at(ball.transform);
	}

	dozers = other.dozers;
	for (auto &dozer : dozers) {
		dozer.transform = transform_to_transform.at(dozer.transform);
	}

	return *this;
}

PoolLevel::Dozer *PoolLevel::add_dozer(std::string const &name, Dozer::Team const &team) {
	//Add drawable to the scene:
	transforms.emplace_back();
	drawables.emplace_back(&transforms.back());
	drawables.back().pipeline = lit_color_texture_program_pipeline;
	drawables.back().pipeline.vao = pool_meshes_for_lit_color_texture_program;
	Mesh const *mesh = (team == Dozer::TeamSolid ? mesh_DozerSolid : mesh_DozerDiamond);
	drawables.back().pipeline.type = mesh->type;
	drawables.back().pipeline.start = mesh->start;
	drawables.back().pipeline.count = mesh->count;

	//Add associated dozer to list:
	dozers.emplace_back();
	dozers.back().transform = &transforms.back();
	dozers.back().name = name;
	dozers.back().team = team;

	return &dozers.back();
}

void PoolLevel::remove_dozer(Dozer *dozer) {
	//find and remove drawable:
	for (auto di = drawables.begin(); di != drawables.end(); ++di) {
		if (di->transform == dozer->transform) {
			drawables.erase(di);
			break;
		}
	}
	//find and remove transform:
	for (auto ti = transforms.begin(); ti != transforms.end(); ++ti) {
		if (&*ti == dozer->transform) {
			transforms.erase(ti);
			break;
		}
	}
	//find and remove dozer:
	for (auto di = dozers.begin(); di != dozers.end(); ++di) {
		if (&*di == dozer) {
			dozers.erase(di);
			break;
		}
	}
}

PoolLevel::Dozer *PoolLevel::spawn_dozer(std::string const &name) {
	//TODO: balance teams
	Dozer::Team team = Dozer::TeamDiamond;

	Dozer *dozer = add_dozer(name, team);
	//TODO: rejection sample center area of level
	dozer->transform->position = glm::vec3(0.0f);

	return dozer;
}


void PoolLevel::update(float elapsed) {
	//move dozers:
	for (auto &d : dozers) {

		float left_target = 0.0f;
		if (d.controls.left_forward) left_target += 1.0f;
		if (d.controls.left_backward) left_target -= 1.0f;

		float right_target = 0.0f;
		if (d.controls.right_forward) right_target += 1.0f;
		if (d.controls.right_backward) right_target -= 1.0f;

		left_target *= 1.0f;
		right_target *= 1.0f;

		if (d.left_tread < left_target) {
			d.left_tread = std::min(d.left_tread + 10.0f * elapsed, left_target);
		} else {
			d.left_tread = std::max(d.left_tread - 10.0f * elapsed, left_target);
		}

		if (d.right_tread < right_target) {
			d.right_tread = std::min(d.right_tread + 10.0f * elapsed, right_target);
		} else {
			d.right_tread = std::max(d.right_tread - 10.0f * elapsed, right_target);
		}

		float angle = glm::roll(d.transform->rotation);

		float speed = elapsed * 0.5f * (d.left_tread + d.right_tread);
		float spin = elapsed * (d.right_tread - d.left_tread) / 0.25f;

		d.transform->position += speed * glm::vec3(std::cos(angle), std::sin(angle), 0.0f);
		d.transform->rotation = glm::angleAxis(angle + spin, glm::vec3(0.0f, 0.0f, 1.0f));
	}

	//push everything so that it no longer intersects:

	auto push_apart = [](glm::vec3 &a, glm::vec3 &b, float radius, float mix = 0.5f) {
		glm::vec2 ab = glm::vec2(b) - glm::vec2(a);
		float len2 = glm::dot(ab, ab);
		if (len2 >= radius*radius) return;
		if (len2 == 0.0f) return;
		float len = std::sqrt(len2);
		ab /= std::sqrt(len);
		a -= glm::vec3((1.0f - mix) * (radius - len) * ab, 0.0f);
		b += glm::vec3(mix * (radius - len) * ab, 0.0f);
	};

	auto keep_in_level = [this](glm::vec3 &a, float radius) {
		a.x = std::min(level_max.x - radius, std::max(level_min.x + radius, a.x));
		a.y = std::min(level_max.y - radius, std::max(level_min.y + radius, a.y));
	};

	for (uint32_t iter = 0; iter < 10; ++iter) {
		for (auto &a : dozers) {
			for (auto &b : dozers) {
				if (&a == &b) break;
				push_apart(a.transform->position,b.transform->position,0.3f);
			}
			keep_in_level(a.transform->position, 0.15f);
		}
	}
	std::vector< glm::vec3 > old_positions;
	old_positions.reserve(balls.size());
	for (auto &b : balls) {
		old_positions.emplace_back(b.transform->position);
	}
	for (uint32_t iter = 0; iter < 10; ++iter) {
		for (auto &b : balls) {
			for (auto &b2 : balls) {
				if (&b == &b2) break;
				push_apart(b.transform->position,b2.transform->position,0.3f);
			}
			for (auto &a : dozers) {
				push_apart(a.transform->position,b.transform->position,0.3f, 1.0f);
			}
			keep_in_level(b.transform->position, 0.15f);
		}
	}
	auto opi = old_positions.begin();
	for (auto &b : balls) {
		glm::vec3 delta = b.transform->position - *opi;
		float len = glm::length(delta);
		if (len != 0.0f) {
			b.transform->rotation = glm::normalize(
				glm::angleAxis(len/0.15f, glm::normalize(glm::vec3(-delta.y, delta.x, 0.0f)))
				* b.transform->rotation
			);
		}
		++opi;
	}
	assert(opi == old_positions.end());


}
