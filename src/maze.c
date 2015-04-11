#include "maze.h"

#define POS(y,x) ((y)*w+(x))

//#define DEBUG() for(int x=0;x<w;x++) {printf("%2i ",maze[POS(y,x)].set);} printf("\n");

BOOL istaken(Cell* maze, int w, int h, int x, int y) {
  return x>=0 && y>=0 && x<w && y<h && !maze[POS(y,x)].v;
}

void rec(Cell* maze, int w, int h, int x, int y) {
  maze[POS(y,x)].v = TRUE;
  int xopts[4] = {-1,1,0,0};
  int yopts[4] = {0,0,-1,1};

  //while we can, pick random options
  while(TRUE) {
    BOOL options[4];
    int optionscount=0;
    //we have to check each iteration what our options are, because the rec()
    //calls below may have changed things
    for(int i=0;i<4;i++) {
      options[i]=istaken(maze,w,h,x+xopts[i],y+yopts[i]);
      if(options[i]) optionscount++;
    }
    //quit if there's nothing to do
    if(optionscount==0) return;

    //make a list of possible directions
    int choices[optionscount];
    int choicecount=0;
    for(int i=0;i<4;i++) {
      if(options[i]) {
        choices[choicecount++] = i;
      }
    }

    //expand in whichever direction we can
    int direction=choices[rand()%choicecount];
    //now direction is the index of the direction we should take
    if(xopts[direction]>0) maze[POS(y,x)].r = 0;
    else if(xopts[direction]<0) maze[POS(y,x-1)].r = 0;
    if(yopts[direction]>0) maze[POS(y,x)].b = 0;
    else if(yopts[direction]<0) maze[POS(y-1,x)].b = 0;
    rec(maze,w,h,x+xopts[direction],y+yopts[direction]);
  }
}

Cell* genmaze(int w, int h) {

  Cell *maze = malloc(w*h*sizeof(*maze));
  maze[0] = (Cell) {1, 1, 0};
  memset(maze, *((char*) maze), w*h*sizeof(*maze));

  rec(maze,w,h,0,0);

  maze[POS(h-1,w-1)].b = 0;

  return maze;
}
