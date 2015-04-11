#include "maze.h"

#define POS(y,x) ((y)*w+(x))

//#define DEBUG() for(int x=0;x<w;x++) {printf("%2i ",maze[POS(y,x)].set);} printf("\n");


    //doubles back over our set, indicated by curpos, and patches it with holes
void addholes(int pos, int setsize, Cell *maze, int curpos) {
  //how many vertical gaps
  int numholes = 1+(setsize<=1?0:rand()%(setsize/2+1));

  //create a boolean array to represent which should have holes
  BOOL holes[setsize];
  memset(holes, TRUE, numholes*sizeof(BOOL));
  memset(holes+numholes, FALSE, (setsize-numholes)*sizeof(BOOL));
  //shuffle it, we can see inductively this is an even shuffle
  for(int i = 1; i < setsize; i++) {
    int r = rand()%(i+1);
    BOOL tmp = holes[i];
    holes[i] = holes[r];
    holes[r] = tmp;
  }
  //now set holes based on our shuffle
  for(int i = 0; i < setsize; i++) {
    maze[curpos+i].b = !holes[i]?1:0;
  }
}

Cell* genmaze(int w, int h) {
  Cell *maze = calloc(w*h, sizeof(*maze));

  for(int y = 0; y < h-1; y++) {
    unsigned char sets[w];
    memset(sets, 0, w*sizeof(*sets));
    //pass the row down (in particular, the set numbers)
    if(y>0) {
      memcpy(maze+POS(y,0), maze+POS(y-1,0), w);
    }

    //zero out set for all rows that have bottom walls
    //also remove bottom walls
    for(int x = 0; x < w; x++) {
      int pos = POS(y,x);
      if(maze[pos].b) {
        maze[pos].set = 0;
        maze[pos].b = 0;
      }
    }

    //skip 0, 0 is unallocated set
    int spos=1;
    //reassign existing sets to 1,2,3...
    //first loop counts up number of sets
    for(int x = 0; x < w; x++) {
      int pos = POS(y,x);
      if(maze[pos].set) {
        if(!sets[maze[pos].set]) {
          spos++;
          sets[maze[pos].set] = spos-1;
        }
      }
    }
    //second loop assigns them to 1,2,3...
    //0 set cells get incremented too
    for(int x = 0; x < w; x++) {
      int pos = POS(y,x);
      if(maze[pos].set) {
        maze[pos].set = sets[maze[pos].set];
      } else {
        maze[pos].set = spos++;
      }
    }

    //keep track of our merges, so we make a vertical bar instead of
    //merging the same one twice
    unsigned char merges[w];
    memset(merges, 0, w*sizeof(*merges));
    int mergelen = 0;
    //add walls randomly and merge sets
    for(int x = 0; x < w-1; x++) {
      int pos = POS(y,x);

      BOOL sameset = maze[pos].set == maze[pos+1].set;
      
      int matchpos = -1;
      if(!sameset) {
        //check if we've merged before
        for(int i=0;i<mergelen;i++) {
          if(merges[i] != maze[pos+1].set) continue;
          matchpos = i;
          break;
        }
        //if we've already merged with this set AND it wasn't the last merge...
        //then force a vertical bar since we've hit ourselves again
        if(matchpos != -1 && matchpos != mergelen) sameset = TRUE;
      }

      //if the two sets are identical, a wall MUST be placed to avoid loops
      int r = maze[pos].r = sameset || (y!=h-1 && rand()%2);
      if(r) {
        memset(merges, 0, w*sizeof(*merges));
        mergelen = 0;
      }
      else {
        //if no match, show that we've merged
        if(matchpos == -1) {
          merges[mergelen++]=maze[pos+1].set;
        }
        //update all the old sets to reflect the merge
        int oldset = maze[pos+1].set;
        maze[pos+1].set=maze[pos].set;
        for(int i=0;i<w;i++) {
          if(maze[POS(y,i)].set==oldset) maze[POS(y,i)].set=maze[pos].set;
        }
      }
    }
    //right side always has walls
    maze[POS(y,w-1)].r = 1;
    //curpos- the index of the start of the most recent set
    //curset- set number; setsize- number of cells in set
    int curpos, curset, setsize;
    
    curpos=POS(y,0), curset=maze[curpos].set, setsize=0;
    //put at least 1 hole in each set, never have 2 adjacent holes
    //have to count up sets...
    for(int x = 0; x < w; x++) {
      int pos = POS(y,x);
      //count up the size of our current set
      if(curset == maze[pos].set) ++setsize;
      else { //once the set we're looking at changes, we need to do work
        addholes(pos, setsize, maze, curpos);
        curpos = POS(y, x);
        curset = maze[curpos].set;
        setsize = 1;
      }
    }
    //since we'll never transition to another set at the end of a row,
    //add holes a the end
    addholes(POS(y, w-1), setsize, maze, curpos);
  }

  //now close up the last row
  memcpy(maze+POS(h-1,0), maze+POS(h-2,0), w);
  for(int x = 0; x < w-1; x++) {
    int pos = POS(h-1,x);
    //only add walls if they're part of the same set and would loop
    maze[pos].r = (maze[pos].set == maze[pos+1].set) && !maze[pos].b;
    //cap off all the floor
    maze[pos].b = 1;
  }

  return maze;
}
