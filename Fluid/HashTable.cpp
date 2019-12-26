#include "HashTable.h"

HashTable::HashTable() {}

HashTable::HashTable(int table_size, float smoothing_factor, int prime) {
  size = table_size;
  table = std::map<int,std::vector<int>>();
  p1 = 73856093;
  p2 = 19349663;
  p3 = 83492791;
  length_factor = smoothing_factor;
  n_h = prime;
}

HashTable::~HashTable() {}

int HashTable::hash(int x_rx, int x_ry, int x_rz) {
  return (x_rx ^ x_ry ^ x_rz) % n_h;
}

void HashTable::insert(glm::vec3 position, int idx) {
  glm::vec3 x_r = glm::vec3(ceil(position.x/length_factor), ceil(position.x/length_factor), ceil(position.z/length_factor));
  int hashed = hash(x_r.x, x_r.y, x_r.z);
  if (!calculated[hashed]) {
    calculated[hashed] = true;
    table[hashed] = std::vector<int>();
  }
  table[hashed].push_back(idx);
}

std::vector<int> HashTable::getValue(glm::vec3 position) {
  glm::vec3 x_r = glm::vec3(ceil(position.x/length_factor), ceil(position.x/length_factor), ceil(position.z/length_factor));
  int hashed = hash(x_r.x, x_r.y, x_r.z);
  return table[hashed];
}