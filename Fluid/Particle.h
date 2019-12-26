#ifndef _PARTICLE_H
#define _PARTICLE_H

#include <glm/glm.hpp>
#include <vector>

class Particle
{
public:
	enum class UpdateMethod : std::int8_t { EulerOrig, EulerSemi, Verlet };

	Particle();
	Particle(glm::vec3 position, glm::vec3 velocity, float siz, glm::vec3 col, float alpha, float lifetime, float mass);
	glm::vec3 pos;
	glm::vec3 prev;
	glm::vec3 vel;
	float size;
	glm::vec3 color;
	float transparency;
	// SHAPE?
	float lifetime;
	float mass;
	float density;
	float pressure;
	glm::vec3 acc;
	std::vector<int> neighs;

private:


};

#endif