#pragma once
#include "global.h"
typedef struct Cell Cell;
struct Cell {
  unsigned char r : 1;
  unsigned char b : 1;
  unsigned char set : 6;
};


Cell* genmaze(int w, int h);

void printmaze(int w, int h, Cell* maze);
