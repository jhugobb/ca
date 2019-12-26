#ifndef _HASH_TABLE_H
#define _HASH_TABLE_H

#include <map>
#include <vector>
#include <glm/glm.hpp>

class HashTable {
  
  public:
    HashTable();
    HashTable(int table_size, float smooth_factor, int prime);
    ~HashTable();
    int hash(int x_rx, int x_ry, int x_rz);
    void insert(glm::vec3 pos, int idx);
    std::vector<int> getValue(glm::vec3 pos);
  
  private:
    int size;
    float length_factor;
    std::map<int, std::vector<int>> table; // hash, index
    std::map<int, bool> calculated; // hash, index
    int p1;
    int p2;
    int p3;
    int n_h;
};

#endif