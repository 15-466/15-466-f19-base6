#include "RollLevel.hpp"
#include "data_path.hpp"
#include "LitColorTextureProgram.hpp"

#include <unordered_set>
#include <unordered_map>
#include <iostream>

//used for lookup later:
Mesh const *mesh_Goal = nullptr;
Mesh const *mesh_Sphere = nullptr;

//names of mesh-to-collider-mesh:
std::unordered_map< Mesh const *, Mesh const * > mesh_to_collider;

GLuint roll_meshes_for_lit_color_texture_program = 0;

//Load the meshes used in Sphere Roll levels:
Load< MeshBuffer > roll_meshes(LoadTagDefault, []() -> MeshBuffer * {
	MeshBuffer *ret = new MeshBuffer(data_path("roll-parts.pnct"));

	//Build vertex array object for the program we're using to shade these meshes:
	roll_meshes_for_lit_color_texture_program = ret->make_vao_for_program(lit_color_texture_program->program);

	//key objects:
	mesh_Goal = &ret->lookup("Goal");
	mesh_Sphere = &ret->lookup("Sphere");
	
	//these meshes collide as (simpler) boxes:
	mesh_to_collider.insert(std::make_pair(&ret->lookup("Block.Dark"), &ret->lookup("Block.Simple")));
	mesh_to_collider.insert(std::make_pair(&ret->lookup("Block.Light"), &ret->lookup("Block.Simple")));

	//these meshes collide as themselves:
	mesh_to_collider.insert(std::make_pair(&ret->lookup("Round.Quarter"), &ret->lookup("Round.Quarter")));
	mesh_to_collider.insert(std::make_pair(&ret->lookup("Round.Corner"), &ret->lookup("Round.Corner")));
	mesh_to_collider.insert(std::make_pair(&ret->lookup("Round.Corner.Outer"), &ret->lookup("Round.Corner.Outer")));

	return ret;
});

//Load sphere roll levels:
Load< std::list< RollLevel > > roll_levels(LoadTagLate, []() -> std::list< RollLevel > * {
	std::list< RollLevel > *ret = new std::list< RollLevel >();
	ret->emplace_back(data_path("roll-level-1.scene"));
	ret->emplace_back(data_path("roll-level-2.scene"));
	ret->emplace_back(data_path("roll-level-3.scene"));
	return ret;
});


//-------- RollLevel ---------

RollLevel::RollLevel(std::string const &scene_file) {
	uint32_t decorations = 0;

	//Load scene (using Scene::load function), building proper associations as needed:
	load(scene_file, [this,&scene_file,&decorations](Scene &, Transform *transform, std::string const &mesh_name){
		Mesh const *mesh = &roll_meshes->lookup(mesh_name);
	
		drawables.emplace_back(transform);
		Drawable::Pipeline &pipeline = drawables.back().pipeline;
		
		//set up drawable to draw mesh from buffer:
		pipeline = lit_color_texture_program_pipeline;
		pipeline.vao = roll_meshes_for_lit_color_texture_program;
		pipeline.type = mesh->type;
		pipeline.start = mesh->start;
		pipeline.count = mesh->count;


		//associate level info with the drawable:
		if (mesh == mesh_Sphere) {
			if (player.transform) {
				throw std::runtime_error("Level '" + scene_file + "' contains more than one Sphere (starting location).");
			}
			player.transform = transform;
		} else if (mesh == mesh_Goal) {
			goals.emplace_back(transform);
		} else {
			auto f = mesh_to_collider.find(mesh);
			if (f != mesh_to_collider.end()) {
				mesh_colliders.emplace_back(transform, *f->second, *roll_meshes);
			} else {
				//just decoration.
				++decorations;
			}
		}
	});

	if (!player.transform) {
		throw std::runtime_error("Level '" + scene_file + "' contains no Sphere (starting location).");
	}

	std::cout << "Level '" << scene_file << "' has "
		<< mesh_colliders.size() << " mesh colliders, "
		<< goals.size() << " goals "
		<< "and " << decorations << " decorations."
		<< std::endl;
	
	//Create player camera:
	transforms.emplace_back();
	cameras.emplace_back(&transforms.back());
	camera = &cameras.back();

	camera->fovy = 60.0f / 180.0f * 3.1415926f;
	camera->near = 0.05f;
}

RollLevel::RollLevel(RollLevel const &other) {
	//copy other's transforms, and remember the mapping between them and the copies:
	std::unordered_map< Transform const *, Transform * > transform_to_transform;
	//null transform maps to itself:
	transform_to_transform.insert(std::make_pair(nullptr, nullptr));

	//Copy transforms and store mapping:
	for (auto const &t : other.transforms) {
		transforms.emplace_back();
		transforms.back().name = t.name;
		transforms.back().position = t.position;
		transforms.back().rotation = t.rotation;
		transforms.back().scale = t.scale;
		transforms.back().parent = t.parent; //will update later

		//store mapping between transforms old and new:
		auto ret = transform_to_transform.insert(std::make_pair(&t, &transforms.back()));
		assert(ret.second);
	}

	//update transform parents:
	for (auto &t : transforms) {
		t.parent = transform_to_transform.at(t.parent);
	}

	//copy other's drawables, updating transform pointers:
	drawables = other.drawables;
	for (auto &d : drawables) {
		d.transform = transform_to_transform.at(d.transform);
	}

	//copy other's cameras, updating transform pointers:
	for (auto const &c : other.cameras) {
		cameras.emplace_back(c);
		cameras.back().transform = transform_to_transform.at(c.transform);

		//update camera pointer when that camera is copied:
		if (&c == other.camera) camera = &cameras.back();
	}

	//copy other's lamps, updating transform pointers:
	lamps = other.lamps;
	for (auto &l : lamps) {
		l.transform = transform_to_transform.at(l.transform);
	}

	//---- level-specific stuff ----
	mesh_colliders = other.mesh_colliders;
	for (auto &c : mesh_colliders) {
		c.transform = transform_to_transform.at(c.transform);
	}

	goals = other.goals;
	for (auto &g : goals) {
		g.transform = transform_to_transform.at(g.transform);
	}

	player = other.player;
	player.transform = transform_to_transform.at(player.transform);
}

