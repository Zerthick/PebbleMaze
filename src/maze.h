#ifndef MAZE_H
#define MAZE_H
#include "global.h"
typedef struct Cell Cell;
struct Cell {
  unsigned char r : 1;
  unsigned char b : 1;
  unsigned char v : 1; //visited
};

Cell* genmaze(int w, int h);

#endif
