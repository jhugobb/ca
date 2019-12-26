#include "Map.h"
#include <algorithm>
#include <iostream>

Map::Map() {
}

void Map::addRow(int size) {
  std::vector<Cell*> v(size);
  layout.push_back(v);
}

std::set<char> Map::getTypesOfMesh() {
  return std::set<char>(types);
}
void Map::addWall(int i, int j) {
  if (layout[i][j] == nullptr) layout[i][j] = new Cell();
  layout[i][j]->setType('w');
  layout[i][j]->setCoords(i, j);
  layout[i][j]->createPlanes(i, j, 'w');
  types.insert('w');
}

void Map::addFloor(int i, int j) {
  if (layout[i][j] == nullptr) layout[i][j] = new Cell();
  layout[i][j]->setType(' ');
  layout[i][j]->setCoords(i, j);
  layout[i][j]->createPlanes(i, j, ' ');
  types.insert(' ');
}

void Map::addPlayer(int i, int j) {
  if (layout[i][j] == nullptr) layout[i][j] = new Cell();
  player_i = i;
  player_j = j;
  layout[i][j]->setType(' ');
  layout[i][j]->setCoords(i, j);
  types.insert(' ');
  layout[i][j]->createPlanes(i, j, ' ');
}

void Map::addModel(int i, int j) {
  if (layout[i][j] == nullptr) layout[i][j] = new Cell();
  layout[i][j]->setType('m');
  layout[i][j]->setCoords(i, j);
  types.insert('m');
  layout[i][j]->createPlanes(i, j, ' ');
}

char Map::getLayoutType(int i, int j) {
  return layout[i][j]->getType();
}

float h(glm::vec2 start, glm::vec2 node, glm::vec2 end) {
  return glm::distance(start, node) + glm::distance(node, end);
}

void reconstruct (std::map<Cell*,Cell*> cameFrom, Cell* current, std::vector<glm::vec2>* path) {
  path->push_back(glm::vec2(current->row, current->col));
  while (cameFrom.find(current) != cameFrom.end()) {
    current = cameFrom[current];
    path->push_back(glm::vec2(current->row, current->col));
  }
}

int operator<(const Node& one, const Node& two) {
  return one.f < two.f;
}

void Map::cleanMap() {
  for (uint i = 0; i < layout.size(); i++) {
    for (uint j = 0; j < layout[i].size(); j++) {
      layout[i][j]->g = std::numeric_limits<float>::max();
      layout[i][j]->f = std::numeric_limits<float>::max();
    }
  }
}

glm::vec2 Map::jump(Cell* neigh, Cell* parent, int lookahead) {
  glm::vec2 parent_coords = glm::vec2(parent->row, parent->col);
  glm::vec2 neigh_coords = glm::vec2(neigh->row, neigh->col);
  glm::vec2 dir = glm::normalize(neigh_coords - parent_coords);
  glm::vec2 possible_node = parent_coords + dir * (float) lookahead;
  possible_node.x = floor(possible_node.x);
  possible_node.y = floor(possible_node.y);

  for (uint i = 1; i < lookahead + 1; i++) {
    uint pos_x = floor(parent_coords.x + dir.x * i);
    uint pos_y = floor(parent_coords.y + dir.y * i);
    if (layout[pos_x][pos_y]->getType() == 'w') 
      return jump(neigh, parent, i-1);
  }

  return possible_node;
}

bool Map::A_star(glm::vec2 start, glm::vec2 end, std::vector<glm::vec2>* path) {
  
  cleanMap();
  std::set<Node> open;
  Cell* start_cell = layout[start.x][start.y];
  start_cell->g = 0;
  start_cell->f = h(start, start, end);
  
  Node start_node = Node(start_cell);
  open.insert(start_node);
  std::map<Cell*, Cell*> cameFrom;
  int iter = 0;
  while (open.size() != 0 && iter < 12080) {
    iter++;
    Node current = *open.begin();
    open.erase(current);
    glm::vec2 current_coords = glm::vec2(current.cell->row, current.cell->col);
    // std::cout << current_coords.x << " " << current_coords.y << " checked" << std::endl;
    if (current_coords == end) {
      reconstruct(cameFrom, current.cell, path);
      // std::cout << "DONE" << std::endl;
      return true;
    }
    for (int i = current.cell->row - 1; i < current.cell->row + 2; i++) {
      for (int j = current.cell->col - 1; j < current.cell->col + 2; j++) {
        if (i == current.cell->row && j == current.cell->col) continue;
        if (layout[i][j]->getType() == 'w') continue;
        Cell* neigh_cell = layout[i][j];
        glm::vec2 neigh_coords = jump(neigh_cell, current.cell, 5);
        // glm::vec2 neigh_coords = jump(glm::vec2(neigh_cell->row,neigh_cell->col), current_coords, 5);
        // if (glm::distance(current_coords, neigh_coords) > 1.0f) continue;
        float cost = current.cell->g + glm::distance(neigh_coords, end);
        if (cost < neigh_cell->g) {
          neigh_cell->g = cost;
          neigh_cell->f = neigh_cell->g + h(start, neigh_coords, end);
          
          Node thisnode = Node(neigh_cell);
          cameFrom[neigh_cell] = current.cell;
          if (open.find(thisnode) == open.end()) {
            open.insert(thisnode);
          }
        }
      }
    }
  }
  return false; 
}