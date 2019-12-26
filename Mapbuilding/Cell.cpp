#include "Cell.h"


Cell::Cell() {
  g = std::numeric_limits<float>::max();
  f = std::numeric_limits<float>::max();
}
Cell::~Cell() {}

void Cell::setType(char t) {
  type_of_object = t;
  rank = 0;
  parent = nullptr;
}

void Cell::setCoords(int i, int j) {
  row = i;
  col = j;
}

char Cell::getType() {
  return type_of_object;
}

void Cell::createPlanes(int i, int j, char type) {
  planes = std::vector<Plane*>();

  glm::vec3 n = glm::vec3(0,1,0);
  Plane* p = new Plane(glm::vec3(i, 0, j), n);
  planes.push_back(p);

  if (type != ' ') {
    glm::vec3 normal1 = glm::vec3(-1, 0, 0);
    Plane* p1 = new Plane(glm::vec3(i-0.5, 0, j), normal1);
    planes.push_back(p1);

    glm::vec3 normal2 = glm::vec3(1, 0, 0);
    Plane* p2 = new Plane(glm::vec3(i+0.5, 0, j), normal2);
    planes.push_back(p2);

    glm::vec3 normal3 = glm::vec3(0, 0, -1);
    Plane* p3 = new Plane(glm::vec3(i, 0, j-0.5), normal3);
    planes.push_back(p3);

    glm::vec3 normal4 = glm::vec3(0, 0, 1);
    Plane* p4 = new Plane(glm::vec3(i, 0, j+0.5), normal4);
    planes.push_back(p4);
  }
}

std::vector<Plane*> Cell::getPlanes() {
  return planes;
}

int Cell::operator<(const Cell &other) {
  return f < other.f;
}