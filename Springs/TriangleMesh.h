#ifndef _TRIANGLE_MESH_INCLUDE
#define _TRIANGLE_MESH_INCLUDE


#include <string>
#include <vector>
#include <glm/glm.hpp>
#include "ShaderProgram.h"
using namespace std;

// Class TriangleMesh renders a very simple room with textures

class TriangleMesh
{

public:
	TriangleMesh();

	void addVertex(const glm::vec3 &position);
	void addTriangle(int v0, int v1, int v2);
	void buildCube(float size, float inside, glm::vec3 position);
	void buildParticle(glm::vec3 position);
	vector<int> getTriangles();
	vector<glm::vec3> getVertices();
	void sendToOpenGL(ShaderProgram &program);
	void render() const;
	vector<glm::vec3> vertices;
	void free();

private:
	vector<GLuint> vaos;
	vector<GLuint> vbos;
	vector<int> triangles;
	vector<GLint> posLocations;
	vector<GLint> normalLocations;
	GLuint vao;
	GLuint vbo;
	GLint posLocation, normalLocation;
	bool is_particle;
	
};


#endif // _TRIANGLE_MESH_INCLUDE


