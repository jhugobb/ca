#ifndef _MAP_INCLUDE
#define _MAP_INCLUDE

#include <vector>
#include <set>
#include <map>
#include <queue>

#include "Cell.h"


struct Node {
  Cell* cell;
  float g;
  float f;

  Node(Cell* c) {
    cell = c;
    g = c->g;
    f = c->f;
  }

  int operator <(const Node &other) {
    return f < other.f;
  }
};

class Map {

  public:
    Map();
    void addWall(int, int);
    void addFloor(int, int);
    void addModel(int, int);
    void addRow(int);
    void addBlank(int, int);
    void addPlayer(int, int);

    bool A_star(glm::vec2 start, glm::vec2 goal, std::vector<glm::vec2>* path);
    char getLayoutType(int, int);
    glm::vec2 jump(Cell* neigh, Cell* parent, int lookahead);
    void cleanMap();
    std::set<char> getTypesOfMesh();
    std::vector<std::vector<Cell*>> layout;

    int player_i;
    int player_j;

  private:
    std::set<char> types;
};
#endif
