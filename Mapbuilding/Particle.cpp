#include "Particle.h"
#include <iostream>

Particle::Particle()
{
}

Particle::Particle(const float& x, const float& y, const float& z) :
m_previousPosition(0, 0, 0), m_velocity(0, 0, 0), m_force(0, 0, 0), m_bouncing(0.8), m_lifetime(5), m_fixed(false)
{
	m_currentPosition.x = x;
	m_currentPosition.y = y;
	m_currentPosition.z = z;
	m_previousPosition = m_currentPosition;
}

/*
Particle::Particle(glm::vec3 pos, glm::vec3 vel, float bouncing, bool fixed, int lifetime, glm::vec3 force) :
m_currentPosition(pos), m_previousPosition(pos), m_force(force), m_velocity(vel), m_bouncing(bouncing), m_lifetime(lifetime), m_fixed(fixed)
{
}
*/

Particle::~Particle()
{
}

//setters
void Particle::setPosition(const float& x, const float& y, const float& z)
{
	glm::vec3 pos(x,y,z);
	m_currentPosition =  pos;
}
void Particle::setPosition(glm::vec3 pos)
{
	m_currentPosition = pos;
}

void Particle::setPreviousPosition(const float& x, const float& y, const float& z)
{
	glm::vec3 pos(x, y, z);
	m_previousPosition = pos;
}

void Particle::setPreviousPosition(glm::vec3 pos)
{
	m_previousPosition = pos;
}

void Particle::setForce(const float& x, const float& y, const float& z)
{
	glm::vec3 force(x, y, z);
	m_force = force;
}

void Particle::setForce(glm::vec3 force)
{
	m_force = force;
}

void Particle::addForce(const float& x, const float& y, const float& z)
{
	glm::vec3 force(x,y,z);
	m_force += force;
}

void Particle::addForce(glm::vec3 force)
{
	m_force += force;
}

void Particle::setVelocity(const float& x, const float& y, const float& z)
{
	glm::vec3 vel(x,y,z);
	m_velocity = vel;
}

void Particle::setVelocity(glm::vec3 vel)
{
	m_velocity = vel;
}

void Particle::setBouncing(float bouncing)
{
	m_bouncing = bouncing;
}

void Particle::setLifetime(float lifetime)
{
	m_lifetime = lifetime;
}

void Particle::setFixed(bool fixed)
{
	m_fixed = fixed;
}

//getters
glm::vec3 Particle::getCurrentPosition()
{
	return m_currentPosition;
}

glm::vec3 Particle::getPreviousPosition()
{
	return m_previousPosition;
}

glm::vec3 Particle::getForce()
{
	return m_force;
}

glm::vec3 Particle::getVelocity()
{
	return m_velocity;
}

float Particle::getBouncing()
{
	return m_bouncing;
}

float Particle::getLifetime()
{
	return m_lifetime;
}

bool Particle::isFixed()
{
	return m_fixed;
}

bool Particle::updateParticle(const float& dt, const glm::vec3 steering, UpdateMethod method)
{
	bool has_reached_goal = false;
	// std::cout << m_currentPosition.x << " " << m_currentPosition.z << " vs " << path->at(path_size)->cell->row << " " << path->at(path_size)->cell->col << std::endl;
	// std::cout << (m_currentPosition.x == path->at(path_size)->cell->row) << " and " << (m_currentPosition.z == path->at(path_size)->cell->col) << std::endl;
	double diff_1 = abs(m_currentPosition.x - path->at(path_size).x);
	double diff_2 = abs(m_currentPosition.z - path->at(path_size).y);
	double epsilon = 0.5;
	if (diff_1 < epsilon && diff_2 < epsilon) {
		path_size = std::max(0, path_size-1);
		if (path_size == 0) has_reached_goal = true;
	}
	// glm::vec3 norm_steering = glm::normalize(steering);
	// norm_steering *= 0.5f;
	m_velocity += steering * dt * 2.0f;
	m_velocity.x = std::min(m_velocity.x, 4.0f);
	m_velocity.y = 0;
	m_velocity.z = std::min(m_velocity.z, 4.0f);

	if (!m_fixed & m_lifetime > 0)
	{
		switch (method)
		{
		case UpdateMethod::EulerOrig:
		{
			m_previousPosition = m_currentPosition;
			m_currentPosition += m_velocity*dt;
			m_velocity += m_force*dt;
		}
			break;
		case UpdateMethod::EulerSemi:
		{
			m_previousPosition = m_currentPosition;
			m_velocity += m_force*dt;
			m_currentPosition += m_velocity*dt;
		}
			break;
		case UpdateMethod::Verlet:
		{
			glm::vec3 new_pos;
			new_pos = m_currentPosition + float(0.99) * (m_currentPosition - m_previousPosition) + m_force * (dt*dt);
			m_velocity = (new_pos - m_currentPosition)/dt;
			m_previousPosition = m_currentPosition;
			m_currentPosition = new_pos;
		}
			break;
		}
	}
	return has_reached_goal;
}

// float rand_f(float a, float b) {
//     return ((b - a) * ((float)rand() / RAND_MAX)) + a;
// }

void Particle::initParticle(float dt, int part_mode, int update_mode, int i, int j, std::vector<glm::vec2>* res) {
	setForce(0, -9.8, 0);
	path = res;
	path_size = res->size()-1;
	glm::vec3 goal = glm::vec3(path->at(path_size).x, 0, path->at(path_size).y);
	glm::vec3 dir = goal - m_currentPosition;
	dir = glm::normalize(dir);
	// setLifetime(rand_f(1,15));
	switch (part_mode) {
	case 0:			
		setPosition(i,0.1,j);
		setVelocity(2.0f * dir);
		break;
	case 1:
		setPosition(i,0.1,j);
		setVelocity(2.0f * dir);
		break;
	}
	if (update_mode == 2) {
		setPreviousPosition(getCurrentPosition() - dt * getVelocity()); 
	}
}

void Particle::update_particle_plane(Plane* plane, int update_mode) {
	glm::vec3 new_pos = getCurrentPosition() - ((1.0f + getBouncing()) * (glm::dot(plane->normal, getCurrentPosition()) + plane->dconst) * plane->normal);
	glm::vec3 normal_vel = glm::dot(plane->normal, getVelocity()) * plane->normal;
	glm::vec3 tangent_vel = getVelocity() - normal_vel;
	glm::vec3 new_vel = getVelocity() - (( (1.0f+getBouncing()) * glm::dot(plane->normal, getVelocity())) * plane->normal);
	new_vel = new_vel - 0.1f * tangent_vel;
	if (update_mode == 2) {
		glm::vec3 corrected = getPreviousPosition() - 2.0f * (glm::dot(plane->normal, getPreviousPosition()) + plane->dconst) * plane->normal;
		setPreviousPosition(corrected);
	}
	setPosition(new_pos);
	setVelocity(new_vel);
}

void Particle::update_particle_sphere(glm::vec3 normal, glm::vec3 point, int update_mode) {
	Plane* plane = new Plane(point, normal);
	glm::vec3 new_pos = getCurrentPosition() - ((1.0f + getBouncing()) * (glm::dot(plane->normal, getCurrentPosition()) + plane->dconst) * plane->normal);
	glm::vec3 normal_vel = glm::dot(plane->normal, getVelocity()) * plane->normal;
	glm::vec3 tangent_vel = getVelocity() - normal_vel;
	glm::vec3 new_vel = getVelocity() - (( (1.0f+getBouncing()) * glm::dot(plane->normal, getVelocity())) * plane->normal);
	new_vel = new_vel - 0.1f * tangent_vel;
	if (update_mode == 2) {
		glm::vec3 corrected = getPreviousPosition() - 2.0f * (glm::dot(plane->normal, getPreviousPosition()) + plane->dconst) * plane->normal;
		setPreviousPosition(corrected);
	}
	setPosition(new_pos);
	setVelocity(new_vel);
	free(plane);
}

void Particle::setNewPath(std::vector<glm::vec2>* newpath) {
	path->clear();
	delete path;
	path = newpath;
	path_size = path->size() - 1;
}

glm::vec3 Particle::getGoal() {
	return glm::vec3(path->at(path_size).x, 0, path->at(path_size).y);
}