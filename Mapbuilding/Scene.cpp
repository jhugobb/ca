#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_access.hpp>
#include <ctime>
#include <iostream>
#include <algorithm>

#include "Scene.h"
#include "PLYReader.h"
#include "Geometry.h"

#define GLM_FORCE_RADIANS
#define MAX_LOOK_AHEAD 1.5f
#define MAX_AVOID_FORCE 1.0f

float rand_f(float a, float b) {
    return ((b - a) * ((float)rand() / RAND_MAX)) + a;
}

Scene::Scene()
{
	map = new Map();
	delta = 0;
	framerate = 0;
}

Scene::~Scene()
{
	meshes.clear();
}

std::vector<std::string> split(const std::string & str, const std::string & delim) {
    std::vector<std::string> tokens;
    unsigned int prev = 0, pos = 0;
    do {
        pos = str.find(delim, prev);
        if (pos == std::string::npos) pos = str.length();
        std::string token = str.substr(prev, pos - prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    } while (pos < str.length() && prev < str.length());
    return tokens;
}

bool Scene::parseVisibility(string filename) {
	std::ifstream file(filename, ios::in);
	if (!file.is_open()) {
		cout << "Visibility file not found" << endl;
		return false;
	}
  std::string line; 
  unsigned int i = 0;
  while (std::getline(file, line)) {
		std::vector<std::string> vals = split(line, " ");
		std::unordered_set<int> PVS;
		for (std::string idx_s : vals) {
			PVS.emplace(std::stoi(idx_s));
		}
		PVSs[i] = PVS;
		i++;
	}
	return true;
}

bool Scene::parse(string filename) {
  std::ifstream file(filename, ios::in);
	if (!file.is_open()) {
		cout << "Map file not found" << endl;
		return false;
	}
  std::string line; 
  unsigned int size;
  unsigned int i = 0;
	
  while (std::getline(file, line)) {
    size = line.size(); 
    map->addRow(size);
    for (unsigned int j= 0; j< size; j++) {
			Particle* p;
      switch (line[j]) {
        case 'w':
          map->addWall(i,j);
					map->layout[i][j]->createPlanes(i, j, 'w');
          break;
				case 'm':
					map->addModel(i,j);
					p = new Particle(i, 0, j);
					particles.push_back(p);
					break;
				case 'p':
				map->addFloor(i,j);
					map->layout[i][j]->createPlanes(i, j, ' ');
					map->addPlayer(i,j);
        default:
          map->addFloor(i,j);
					map->layout[i][j]->createPlanes(i, j, ' ');
          break;
      }
    }
    i++;
  }
	file.close();
	return true;
}

bool Scene::init(int argc, char* argv[])
{
	initShaders();

	vector<string> args;
	for (int i = 0; i < argc; i++) {
		args.push_back(string(argv[i]));
	}

	if (args.size() != 3) {
		cout << "Usage: ./BaseCode file.map file.vis" << endl;
		cout << "Now parsing ../maps/3.map and ../visibility/results/3.vis" << endl;
		if (!parse( "../maps/3.map")) {
			return false;
		}
		if (!parseVisibility("../visibility/results/3.vis")) {
			return false;
		}
	} else {
		if (!parse(args[1])) return false;
		if (!parseVisibility(args[2])) return false;
	}
	// Parse the map
	// Parse the visibility

	vector<char> possibilities = {'a', 'b', 'd', 'f', 'h', 'l',
																'w', 'm', 'o', 'r'};
	std::map<char, bool> is_comp;
	for (char c : possibilities) is_comp[c] = false;

	bool is_mesh;
	std::string mesh_location;
	
	// Build mesh instances
	for (char c : map->getTypesOfMesh()) {
		TriangleMesh* mesh = new TriangleMesh();
		bool success;
		PLYReader reader;
		if (is_comp[c]) {
			free(mesh);
			continue;
		}
		is_mesh = false;
		switch (c) {
			case 'w':
				mesh->buildCube(0.5f);
				mesh->sendToOpenGL(basicProgram, is_mesh);
				meshes.insert(std::pair<char,TriangleMesh*>(c,mesh));
				is_comp['w'] = true;
				break;
			case ' ':
				mesh->buildFloor();
				mesh->sendToOpenGL(basicProgram, is_mesh);
				meshes.insert(std::pair<char,TriangleMesh*>(c,mesh));
				is_comp[' '] = true;
				break;
			case 'm':
				mesh->buildCube(0.1f);
				mesh->sendToOpenGL(basicProgram, is_mesh);
				meshes.insert(std::pair<char,TriangleMesh*>(c,mesh));
				is_comp['m'] = true;
				break;
			default:
				break;
		}
		if (is_mesh) {
			cout << "Reading " << mesh_location << " file." << endl;
			success = reader.readMesh(mesh_location, *mesh);
			if (!success) continue;
			mesh->sendToOpenGL(basicProgram, is_mesh);
			meshes.insert(std::pair<char,TriangleMesh*>(c,mesh));
			is_comp[c] = true;
		}
	}

	currentTime = 0.0f;
	
	camera.init(2.0f);
	// Set player position
	camera.setPlayer(-map->player_i, -map->player_j);
	
	bPolygonFill = true;
	vis_mode = false;

	for (Particle* p : particles) {
		glm::vec2 newgoal = glm::vec2(floor(rand_f(0,map->layout.size()-1)), floor(rand_f(0,map->layout[0].size()-1)));
		while (map->layout[newgoal.x][newgoal.y]->getType() != ' ' && newgoal != glm::vec2(p->m_currentPosition.x, p->m_currentPosition.z)) {
			newgoal = glm::vec2(floor(rand_f(0,map->layout.size()-1)), floor(rand_f(0,map->layout[0].size())-1));
		}
		std::vector<glm::vec2>* newpath = new std::vector<glm::vec2>();

		glm::vec2 start = glm::vec2(floor(p->m_currentPosition.x), floor(p->m_currentPosition.z));
		bool done = false;
		while(!done) {
			delete newpath;
			newpath = new std::vector<glm::vec2>();
			done = map->A_star(start, newgoal, newpath);
		}

		cout << "new goal " << newgoal.x << " " << newgoal.y << endl;

		if (newpath->size() != 0) {
			newpath->pop_back();
			p->initParticle(0, 1, 1, start.x, start.y, newpath);
		}
	}

	// Select which font you want to use
	if(!text.init("fonts/OpenSans-Regular.ttf"))
	//if(!text.init("fonts/OpenSans-Bold.ttf"))
	//if(!text.init("fonts/DroidSerif.ttf"))
		cout << "Could not load font!!!" << endl;
	return true;
}

bool Scene::loadMesh(const char *filename)
{
	return false;
}

glm::vec3 Scene::get_closest_position_of_obstacle(Particle* p, glm::vec3 position, bool only_consider_agents) {
	int x = floor(position.x);
	int y = floor(position.z);
	glm::vec3 out = glm::vec3(-1, 0, -1);
	float mindist = std::numeric_limits<float>::max();

	if (!only_consider_agents) {
		for (int i = x - 1; i < x + 2; i++) {
			for (int j = y - 1; j < y + 2; j++) {
				if (i == x && j == y) continue;
				float curr_dist = glm::distance(position, glm::vec3(i, 0, j));
				if (map->layout[i][j]->getType() == 'w' && curr_dist < mindist && curr_dist < 1.41f) {
					mindist = curr_dist;
					out = glm::vec3(i,0,j);
				}
			}
		}
	} else {
		for (Particle *p2 : particles) {
			if (p2 == p) continue;
			float curr_dist = glm::distance(position, p2->getCurrentPosition());
			if (curr_dist < mindist && curr_dist < 1.41f) {
				mindist = curr_dist;
				out = p2->getCurrentPosition();
			}
		}
	}	
	
	return out;
}


glm::vec3 Scene::get_avoidance(Particle* p) {
	glm::vec3 ahead = p->getCurrentPosition() + glm::normalize(p->getVelocity()) * MAX_LOOK_AHEAD;
	glm::vec3 ahead2 = p->getCurrentPosition() + glm::normalize(p->getVelocity()) * MAX_LOOK_AHEAD * 0.5f;
	glm::vec3 out = glm::vec3(0,0,0);
	glm::vec3 closest_wall = get_closest_position_of_obstacle(p, p->getCurrentPosition(), false);
	// cout << "Closest obstacle " << closest_wall.x << " " << closest_wall.z << endl;
	// cout << "Position ahead " << ahead.x << " " << ahead.z << endl;

	// The radius of every object is sqrt(2)
	if (closest_wall.x != -1 && (glm::distance(ahead, closest_wall) <= 1.41f || glm::distance(ahead2, closest_wall) <= 1.41f)) {
		glm::vec3 avoidance_force_w = ahead - closest_wall;
		avoidance_force_w = normalize(avoidance_force_w) * 1.0f/glm::distance(p->getCurrentPosition(), closest_wall);
		out += avoidance_force_w;
	}

	glm::vec3 closest_agent = get_closest_position_of_obstacle(p, p->getCurrentPosition(), true);
	// cout << "Closest obstacle " << closest_obstacle.x << " " << closest_obstacle.z << endl;
	// cout << "Position ahead " << ahead.x << " " << ahead.z << endl;

	// The radius of every object is sqrt(2)
	if (closest_agent.x != -1 && (glm::distance(ahead, closest_agent) <= 1.41f || glm::distance(ahead2, closest_agent) <= 1.41f)) {
		glm::vec3 avoidance_force_a = ahead - closest_agent;
		avoidance_force_a = normalize(avoidance_force_a) * 1.0f/glm::distance(p->getCurrentPosition(), closest_agent);
		out += avoidance_force_a;
	}

	return out;
}

glm::vec3 Scene::get_seek_force(Particle* p) {
	glm::vec3 goal = p->getGoal();
	glm::vec3 dir = goal - p->getCurrentPosition();
	dir = glm::normalize(dir);
	return dir; 
}

void Scene::update(int deltaTime, bool forward, bool back, bool left, bool right, bool bPlay) 
{
	if (!bPlay) return;
	currentTime += deltaTime;
	framerate = 1.0f/deltaTime * 1000;
	
	glm::mat4 m;
	m = camera.getModelViewMatrix();
	glm::vec3 mov(0);
	// Get forward vector of camera
	glm::vec3 forw;
	if (!vis_mode) forw = glm::normalize(glm::vec3(m[0][2], 0, m[2][2]));
	else forw = glm::normalize(glm::vec3(m[0][2], m[1][2], m[2][2]));
	glm::vec3 side = glm::normalize(glm::cross(forw, glm::vec3(0,1,0)));
	float rate = float(1.0f/deltaTime);
	if (forward){
		mov += rate * forw;
	} 
	if (back){
		mov -= rate * forw;
	}
	if (left){
		mov -= rate * side;
	} 
	if (right){
		mov += rate * side;
	} 
	camera.move(mov);
	uint update_mode = 1;
	for (unsigned int i = 0; i < particles.size(); i++) {
		Particle* p = particles[i];

		glm::vec3 prev_pos = p->m_currentPosition;

		// Get seek position
		// Get Avoid position
		glm::vec3 total_steering = glm::vec3(0,0,0);

		total_steering += get_seek_force(p);
		total_steering += get_avoidance(p);

		// cout << "current pos" << prev_pos.x << " " << prev_pos.y << " " << prev_pos.z << endl;
		bool needs_new_goal = false;
		switch (update_mode) {
			case 0: // Euler Original
				needs_new_goal = p->updateParticle(deltaTime/1000.0f, total_steering, Particle::UpdateMethod::EulerOrig);
				break;
			case 1: // Euler Semi-Implicit
				needs_new_goal = p->updateParticle(deltaTime/1000.0f, total_steering, Particle::UpdateMethod::EulerSemi);
				break;
			// case 2: // Verlet
			// 	needs_new_goal = p->updateParticle(deltaTime/1000.0f, Particle::UpdateMethod::Verlet, total_steering);
			// 	break;
		}

		// p->setLifetime(p->getLifetime() - deltaTime/1000.0f);
		glm::vec3 coords = p->m_currentPosition;
		int x = std::min(round(coords.x),(float) map->layout.size());
		int y = std::min(round(coords.z), (float)map->layout[0].size());
		std::vector<Plane*> planes = map->layout[x][y]->getPlanes();
		// std::vector<Plane*> planes = map->layout[21][5]->getPlanes();

		for (unsigned int j = 0; j < planes.size(); j++) {
			Plane* plane = planes[j];
			float prev_plane_dist = plane->distPoint2Plane(prev_pos);
			float curr_plane_dist = plane->distPoint2Plane(coords);
			if (prev_plane_dist*curr_plane_dist < 0.0f) { 
				p->update_particle_plane(	plane, update_mode);
			}
		}

		
		if (needs_new_goal) {
			glm::vec2 newgoal;
			std::vector<glm::vec2>* newpath = new std::vector<glm::vec2>();

			bool done = false;
			while(!done) {
				while (needs_new_goal && map->layout[newgoal.x][newgoal.y]->getType() != ' ' && newgoal != glm::vec2(p->m_currentPosition.x, p->m_currentPosition.z)) {
					newgoal = glm::vec2(floor(rand_f(1,map->layout.size()-1)), floor(rand_f(1,map->layout[0].size())-1));
				}
				delete newpath;
				newpath = new std::vector<glm::vec2>();
				done = map->A_star(glm::vec2(floor(p->m_currentPosition.x), floor(p->m_currentPosition.z)), newgoal, newpath);
				if (!done) newgoal = glm::vec2(0,0);
			}

			cout << "new goal " << newgoal.x << " " << newgoal.y << endl;
			if (newpath->size() != 0) {
				newpath->pop_back();
				p->setNewPath(newpath);
			}
		} 
		// cout << prev_pos.x << " " << prev_pos.y << " " << prev_pos.z << endl;
	}
	
}

void Scene::render()
{
	glm::mat3 normalMatrix;

	basicProgram.use();
	glm::mat4 MV = camera.getModelViewMatrix();
	basicProgram.setUniformMatrix4f("projection", camera.getProjectionMatrix());
	normalMatrix = glm::inverseTranspose(camera.getModelViewMatrix());
	basicProgram.setUniformMatrix3f("normalMatrix", normalMatrix);
	glm::vec3 player_pos = camera.getPosition();
	int player_i = -floor(player_pos[0]);
	int player_j = -floor(player_pos[2]);
	basicProgram.setUniform1i("bLighting", bPolygonFill?1:0);
	int size_col = map->layout[0].size();
	int s_row = map->layout.size();
	// tcr.getBestLODs(map, meshes, framerate, delta, PVSs[player_i*size_col + player_j], player_pos);

	basicProgram.setUniform4f("color", 0.9f, 0.9f, 0.95f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	
	// Iterate over the visible map cells from the player POV
	if (!vis_mode) {
		for (int i : PVSs[player_i*size_col + player_j]) {
			int row = i / size_col;
			int col = i % size_col;
			
			if (map->getLayoutType(row,col) == 'm') {
				glm::mat4 matrix = glm::translate(glm::mat4(1.0f), glm::vec3(row, 0, col));
				glm::mat4 mat = MV * matrix;
				basicProgram.setUniformMatrix4f("modelview", mat);
				TriangleMesh * floor = meshes[' '];
				basicProgram.setUniform4f("color", 0, 0.27, 0.4, 1.0);
				floor->render();
			} else {
				// Change color depending on the object
				if (map->getLayoutType(row,col) == ' ') basicProgram.setUniform4f("color", 0.40, 0.27, 0.00, 1);
				else if (map->getLayoutType(row,col) == 'w') basicProgram.setUniform4f("color", 0.68, 0.68, 0.68, 1);
				else basicProgram.setUniform4f("color", 0.9f, 0.9f, 0.95f, 1.0f);
				
				// Translate instance to position
				glm::mat4 matrix = glm::translate(glm::mat4(1.0f), glm::vec3(row, 0, col));
				glm::mat4 mat = MV * matrix;
				basicProgram.setUniformMatrix4f("modelview", mat);
				
				TriangleMesh* m = meshes[map->getLayoutType(row,col)];

				// Render a mesh from a different color depending on its LOD
				if (map->getLayoutType(row,col) != ' ' && map->getLayoutType(row,col) != 'w') {
					basicProgram.setUniform4f("color", 0.9, 0.9, 0.9, 1.0);
				}
				m->render();
				// Draw a floor if the object was a mesh
				if (map->getLayoutType(row,col) != ' ' && map->getLayoutType(row,col) != 'w') {
						TriangleMesh * floor = meshes[' '];
						basicProgram.setUniform4f("color", 0, 0.27, 0.4, 1.0);
						floor->render();
				}
			}
			
		}
	} else {
		for (int i = 0; i<s_row; i++) {
			int s = map->layout[i].size();
			for (int j = 0; j<s; j++) {
				TriangleMesh* m = meshes[map->getLayoutType(i,j)];
				if (PVSs[player_i*s + player_j].find(i*s + j) == PVSs[player_i*s + player_j].end()) {
					basicProgram.setUniform4f("color", 0, 1, 0, 1);
				} else basicProgram.setUniform4f("color", 1, 0, 0, 1);

				if(m) {
					glm::mat4 matrix = glm::translate(glm::mat4(1.0f), glm::vec3(i, 0, j));
					glm::mat4 mat = MV * matrix;
					basicProgram.setUniformMatrix4f("modelview", mat);
					m->render();
				}
				if (map->getLayoutType(i,j) != ' ' && map->getLayoutType(i,j) != 'w') {
						TriangleMesh * floor = meshes[' '];
						floor->render();
				}
			}
		}
	}

	for (Particle* p : particles) {
		TriangleMesh * m = meshes['m'];
		glm::mat4 matrix = glm::translate(glm::mat4(1.0f), p->m_currentPosition);
		glm::mat4 mat = MV * matrix;
		basicProgram.setUniformMatrix4f("modelview", mat);
		basicProgram.setUniform4f("color", 0, 0.27, 0.4, 1.0);
		m->render();

	}

	text.render(std::to_string(framerate), glm::vec2(20, 20), 16, glm::vec4(0, 0, 0, 1));
	
	//cout << "Time render map: " << (end-begin) / CLOCKS_PER_SEC << endl;
}

Camera &Scene::getCamera()
{
  return camera;
}

void Scene::switchPolygonMode()
{
  bPolygonFill = !bPolygonFill;
}

void Scene::initShaders()
{
	Shader vShader, fShader;

	vShader.initFromFile(VERTEX_SHADER, "shaders/basic.vert");
	if(!vShader.isCompiled())
	{
		cout << "Vertex Shader Error" << endl;
		cout << "" << vShader.log() << endl << endl;
	}
	fShader.initFromFile(FRAGMENT_SHADER, "shaders/basic.frag");
	if(!fShader.isCompiled())
	{
		cout << "Fragment Shader Error" << endl;
		cout << "" << fShader.log() << endl << endl;
	}
	basicProgram.init();
	basicProgram.addShader(vShader);
	basicProgram.addShader(fShader);
	basicProgram.link();
	if(!basicProgram.isLinked())
	{
		cout << "Shader Linking Error" << endl;
		cout << "" << basicProgram.log() << endl << endl;
	}
	basicProgram.bindFragmentOutput("outColor");
	vShader.free();
	fShader.free();
}

void Scene::cleanup() {
	for (auto el : meshes) 
		free(el.second);
}

void Scene::setDelta(double del) {
	delta = del;
}

void Scene::toggleVisMode() {
	vis_mode = !vis_mode;
}