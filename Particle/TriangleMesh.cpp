#include <iostream>
#include <vector>
#include "TriangleMesh.h"


using namespace std;


TriangleMesh::TriangleMesh()
{
	is_particle = false;
}

void TriangleMesh::addVertex(const glm::vec3 &position)
{
  vertices.push_back(position);
}

void TriangleMesh::addTriangle(int v0, int v1, int v2)
{
  triangles.push_back(v0);
  triangles.push_back(v1);
  triangles.push_back(v2);
}

void TriangleMesh::buildCube(float size, float inside, glm::vec3 position)
{
	is_particle = false;
	float vertices[] = {-1, -1, -1, // 0
								1, -1, -1,  // 1
								1,  1, -1,  // 2
								-1,  1, -1, // 3
								-1, -1,  1, // 4
								1, -1,  1,  // 5
								1,  1,  1,  // 6
								-1,  1,  1  // 7
	};

	if (inside) {
		int faces[] = {1, 3, 0, 2, 3, 1,
							6, 5, 7, 5, 4, 7,
							3, 7, 0, 4, 0, 7,
							2, 1, 6, 5, 6, 1,
							1, 0, 4, 4, 5, 1,
							3, 2, 7, 6, 7, 2
		};
		int i;

		for(i=0; i<8; i+=1)
			addVertex(position + (size * glm::vec3(vertices[3*i], vertices[3*i+1], vertices[3*i+2])));
		for(i=0; i<12; i++)
			addTriangle(faces[3*i], faces[3*i+1], faces[3*i+2]);
	} else {
		int faces[] = {3, 1, 0, 3, 2, 1,
                 5, 6, 7, 4, 5, 7,
                 7, 3, 0, 0, 4, 7,
                 1, 2, 6, 6, 5, 1,
                 0, 1, 4, 5, 4, 1,
                 2, 3, 7, 7, 6, 2
		};
		int i;

		for(i=0; i<8; i+=1)
			addVertex(size * glm::vec3(vertices[3*i], vertices[3*i+1], vertices[3*i+2]));
		for(i=0; i<12; i++)
			addTriangle(faces[3*i], faces[3*i+1], faces[3*i+2]);
	}
}

void TriangleMesh::buildParticle(glm::vec3 position) {
	is_particle = true;
	addVertex(0.5f * position);
}

void TriangleMesh::sendToOpenGL(ShaderProgram &program) {

	if (!is_particle) {
		// Set up mesh and attribute properties
		vector<float> data;

		for(unsigned int tri=0; tri<triangles.size(); tri+=3)
		{
			glm::vec3 normal;
			
			normal = glm::cross(vertices[triangles[tri+1]] - vertices[triangles[tri]], 
													vertices[triangles[tri+2]] - vertices[triangles[tri]]);
			normal = glm::normalize(normal);
			for(unsigned int vrtx=0; vrtx<3; vrtx++)
			{
				data.push_back(vertices[triangles[tri + vrtx]].x);
				data.push_back(vertices[triangles[tri + vrtx]].y);
				data.push_back(vertices[triangles[tri + vrtx]].z);

				data.push_back(normal.x);
				data.push_back(normal.y);
				data.push_back(normal.z);
			}
		}

		// Send data to OpenGL
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
		posLocation = program.bindVertexAttribute("position", 3, 6*sizeof(float), 0);
		normalLocation = program.bindVertexAttribute("normal", 3, 6*sizeof(float), (void *)(3*sizeof(float)));
	} else {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float), &vertices[0], GL_STATIC_DRAW);
		posLocation = program.bindVertexAttribute("position", 3, 6*sizeof(float), 0);
	}
}

void TriangleMesh::render() const
{
	glBindVertexArray(vao);
	glEnableVertexAttribArray(posLocation);
	if (is_particle) {
		glDrawArrays(GL_POINTS, 0, 1);
	} else {
		glEnableVertexAttribArray(normalLocation);
		glDrawArrays(GL_TRIANGLES, 0, 3 * 2 * 3 * triangles.size() / 3);
	}
		
}

void TriangleMesh::free()
{
	glDeleteBuffers(1, &vbo);
	glDeleteVertexArrays(1, &vao);
}

vector<glm::vec3> TriangleMesh::getVertices() {
	return vertices;
}

vector<int> TriangleMesh::getTriangles() {
	return triangles;
}
