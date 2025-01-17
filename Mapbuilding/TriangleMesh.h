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

	void buildCube(float size);
	void buildFloor();
	vector<glm::vec3> getVertices();
	vector<int> getTriangles();
	void sendToOpenGL(ShaderProgram &program, bool is_mesh);
	void render() const;
	void free();
	int LOD_level;

private:
  vector<glm::vec3> vertices;
  vector<int> triangles;
	vector<GLuint> vaos;
	vector<GLuint> vbos;
	vector<GLint> posLocations;
	vector<GLint> normalLocations;
	GLuint vao;
	GLuint vbo;
	GLint posLocation, normalLocation;
	bool is_mesh;
	
};


#endif // _TRIANGLE_MESH_INCLUDE


