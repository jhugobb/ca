#ifndef _CELL_INCLUDE
#define _CELL_INCLUDE

#include <vector>
#include "Geometry.h"

class Cell {

  public:
    Cell();
    ~Cell();
    void setType(char);
    void setCoords(int, int);
    void createPlanes(int, int, char);
    char getType();
    std::vector<Plane*> getPlanes();
    int operator<(const Cell& other);
    float rank;
    int row;
    int col;

    Cell* parent;
    float g;
    float f;

  private:
    char type_of_object;
    bool is_obstacle;
    std::vector<Plane*> planes;
};

#endif