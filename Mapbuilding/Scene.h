#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <glm/glm.hpp>
#include "Camera.h"
#include "ShaderProgram.h"
#include "TriangleMesh.h"
#include "Text.h"
#include "Map.h"
#include "Particle.h"

#include <map>
#include <fstream>
#include <string>
#include <unordered_set>

// Scene contains all the entities of our game.
// It is responsible for updating and render them.

class Scene
{

public:
	Scene();
	~Scene();

	bool init(int, char**);
	bool loadMesh(const char *filename);
	void update(int deltaTime, bool, bool, bool, bool, bool);
	void render();
	void setDelta(double);
	void cleanup();
	void toggleVisMode();

  Camera &getCamera();
  
  void switchPolygonMode();

private:
	void initShaders();
	void computeModelViewMatrix();
	bool parse(std::string);
	bool parseVisibility(std::string);
  glm::vec3 get_avoidance(Particle* p);
	glm::vec3 get_closest_position_of_obstacle(Particle* p, glm::vec3 position, bool only_consider_agents);
	glm::vec3 get_seek_force (Particle* p);

private:
  Camera camera;
	std::map<char, TriangleMesh*> meshes;
	std::map<int, unordered_set<int>> PVSs;
	Map *map;
	bool vis_mode;
	double delta;
	float framerate;
	ShaderProgram basicProgram;
	float currentTime;
	std::vector<Particle*> particles;
	bool bPolygonFill;
	
	Text text;

};


#endif // _SCENE_INCLUDE

