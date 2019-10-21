#ifndef _SCENE_INCLUDE
#define _SCENE_INCLUDE


#include <glm/glm.hpp>
#include "Camera.h"
#include "ShaderProgram.h"
#include "TriangleMesh.h"
#include "Text.h"
#include "Geometry.h"
#include "Particle.h"
#include <map>
#include <fstream>
#include <string>
#include <vector>
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
	void toggleParticleMode();
	void toggleUpdateMode();

  Camera &getCamera();
  
  void switchPolygonMode();

private:
	void initShaders();
	void computeModelViewMatrix();
	bool parse(std::string);
	bool parseVisibility(std::string);
	void init_particle(Particle* p, float dt);

private:
  Camera camera;
	std::vector<Particle*> particles;
	std::vector<Plane*> planes;
	std::vector<Triangle*> triangles;
	std::vector<Sphere*> spheres;
	std::vector<std::vector<float>> curr_tri_dist;
	std::vector<std::vector<float>> prev_tri_dist; 
	std::vector<std::vector<float>> curr_plane_dist;
	std::vector<std::vector<float>> prev_plane_dist; 
	std::vector<std::vector<bool>> curr_sphere_dist;
	std::vector<std::vector<bool>> prev_sphere_dist; 
	std::vector<glm::vec4> colors;
	int part_mode;
	int update_mode;
	TriangleMesh* particle_mesh;
	TriangleMesh* cave;
	std::vector<TriangleMesh*> tri_meshes;
	TriangleMesh* sphere_mesh;
	double delta;
	float framerate;
	ShaderProgram basicProgram;
	float currentTime;
	
	bool bPolygonFill;
	
	Text text;

};


#endif // _SCENE_INCLUDE

