#include "Particle.h"

Particle::Particle(glm::vec3 position, glm::vec3 velocity, float siz, glm::vec3 col, float alpha, float l, float m) {
  this->pos = position;
  this->vel = velocity;
  this->size = siz;
  this->color = color;
  this->transparency = alpha;
  this->lifetime = l;
  this->mass = m;
}