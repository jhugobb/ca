#ifndef _PARTICLE_H
#define _PARTICLE_H

#include <glm/glm.hpp>
#include "Geometry.h"
#include "Map.h"

class Particle
{
public:
	enum class UpdateMethod : std::int8_t { EulerOrig, EulerSemi, Verlet };

	Particle();
	Particle(const float& x, const float& y, const float& z);
//	Particle(glm::vec3 pos, glm::vec3 vel, float bouncing = 1.0f, bool fixed = false, int lifetime = -1, glm::vec3 force = glm::vec3(0, 0, 0));
	~Particle();
	//setters
	void setPosition(const float& x, const float& y, const float& z);
	void setPosition(glm::vec3 pos);
	void setPreviousPosition(const float& x, const float& y, const float& z);
	void setPreviousPosition(glm::vec3 pos);
	void setVelocity(const float& x, const float& y, const float& z);
	void setVelocity(glm::vec3 vel);
	void setForce(const float& x, const float& y, const float& z);
	void setForce(glm::vec3 force);
	void setBouncing(float bouncing);
	void setLifetime(float lifetime);
	void setFixed(bool fixed);
	
	//getters
	glm::vec3 getCurrentPosition();
	glm::vec3 getPreviousPosition();
	glm::vec3 getForce();
	glm::vec3 getVelocity();
	glm::vec3 getGoal();
	float getBouncing();
	float getLifetime();
	bool isFixed();

	//other
	void addForce(glm::vec3 force);
	void addForce(const float& x, const float& y, const float& z);
	bool updateParticle(const float& dt, const glm::vec3 steering, UpdateMethod method = UpdateMethod::EulerSemi);
	void initParticle(float dt, int part_mode, int update_mode, int i, int j, std::vector<glm::vec2>* path);
	void update_particle_plane(Plane* plane, int update_mode);
	void update_particle_sphere(glm::vec3 normal, glm::vec3 point, int update_mode);
	void setNewPath(std::vector<glm::vec2>* newpath);

	glm::vec3 m_currentPosition;

private:
	glm::vec3 m_previousPosition;
	glm::vec3 m_force;
	glm::vec3 m_velocity;
	std::vector<glm::vec2>* path;
	int path_size;
	float m_bouncing;
	float m_lifetime;
	bool  m_fixed;

};

#endif