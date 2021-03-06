/*
 * main.c
 * Constructs a Window housing an output TextLayer to show data from 
 * either modes of operation of the accelerometer.
 */

#include <pebble.h>
#include "maze.h"

static BOOL playing = FALSE;
static int difficulty = 2;

#define getPOS(y,x,w) ((y)*w+(x))
static Window *s_main_window;
static TextLayer *s_title_layer;
static bool inverted = false;
static TextLayer *s_bplay_layer;
static TextLayer *s_diff_layer;
static Layer *s_maze_layer;
static Layer *s_player_layer;

static Cell *maze;
static int playerX,dx=0;
static int playerY,dy=0;
static int mazeWidth;
static int mazeHeight;
static int corridorSize;

const int lightdelay = 2500;
static AppTimer* lighttimer = NULL;

static void godark(void* data) {
  light_enable(FALSE);
}
//lights up the screen briefly
static void lightup() {
  light_enable(TRUE);
  if(!app_timer_reschedule(lighttimer, lightdelay)) {
    lighttimer = app_timer_register(lightdelay, godark, NULL);
  }
}

static void load(int w, int h, int cs) {
  dx=dy=0;
  free(maze);
  maze = genmaze(mazeWidth=w, mazeHeight=h);
  corridorSize=cs;
  layer_mark_dirty(s_maze_layer);
  playerX = playerY = 0;
  layer_mark_dirty(s_player_layer);
}

static void data_handler(void* out) {
  app_timer_register(30, data_handler, NULL);
  layer_mark_dirty(s_player_layer);
  
  
  //you win
  if (playerY == mazeHeight-1 && playerX == mazeWidth-1 && dx==0 && dy==0){
    vibes_short_pulse();
    load(mazeWidth,mazeHeight,corridorSize);
    return;
  }
  
  int speed = difficulty==1?2:1;
  if(dx!=0 || dy!=0) {
    if(dx<0) dx+=speed;
    if(dx>0) dx-=speed;
    if(dy<0) dy+=speed;
    if(dy>0) dy-=speed;
    
    return;
  }
  
  AccelData* data = malloc(sizeof(*data));
  accel_service_peek(data);
  

  if(data->x*data->x + data->y*data->y > 150*150) {
    //left, right, up, down, just like maze.c
    int xopts[4] = {-1,1,0,0};
    int yopts[4] = {0,0,-1,1};
    int bestopt=0, bestscore=0;
    int xmag = abs(data->x),
        ymag = abs(data->y);
    //app_log(APP_LOG_LEVEL_INFO,"main.c",1337,"mag %i %i",xmag,ymag);
    for(int i=0; i<4; i++) {
      int newx = playerX+xopts[i],
          newy = playerY+yopts[i];
      //bounds checking
      int curscore = (newx>=0 && newy>=0 && newx<mazeWidth && newy < mazeHeight) ? 1 : 0;
      //wall checking
      if(i<2) {//x
        curscore *= 1-maze[getPOS(playerY,(xopts[i]==1?playerX:newx),mazeWidth)].r;
        curscore *= xmag<50?0:xopts[i]*data->x;
      } else {//y
        curscore *= 1-maze[getPOS((yopts[i]==1?playerY:newy),playerX,mazeWidth)].b;
        curscore *= ymag<50?0:-yopts[i]*data->y;
      }
      //app_log(APP_LOG_LEVEL_INFO,"main.c",1338,"score %i %i",i,curscore);
      if(curscore > bestscore) {
        bestopt = i;
        bestscore = curscore;
      }
    }
    if(bestscore>0) {
      lightup();
      playerX += xopts[bestopt];
      playerY += yopts[bestopt];
      dx -= xopts[bestopt]*(corridorSize-speed);
      dy -= yopts[bestopt]*(corridorSize-speed);
    }
  }
  free(data);
}

static void maze_layer_update_callback(Layer *layer, GContext *ctx) {
  for(int i = 0; i < mazeWidth; i++){
    for(int j = 0; j < mazeHeight; j++){
      int currentPos = getPOS(j, i, mazeWidth);
      if(maze[currentPos].r == 1) {
        graphics_draw_line(ctx, GPoint(corridorSize*(i + 1), corridorSize*j), GPoint(corridorSize*(i + 1), corridorSize*(j + 1)));
      }
      if(maze[currentPos].b == 1) {
        graphics_draw_line(ctx, GPoint(corridorSize*i, corridorSize*(j + 1)), GPoint(corridorSize*(i + 1), corridorSize*(j + 1)));
      }
    }
  }
  graphics_draw_circle(ctx, GPoint((mazeWidth-1)*corridorSize+corridorSize/2,
                                   (mazeHeight-1)*corridorSize+corridorSize/2), corridorSize/2-1);
}

static void player_layer_update_callback(Layer *layer, GContext *ctx) {
    graphics_fill_circle(ctx, GPoint(playerX*corridorSize+dx+corridorSize/2,
                                     playerY*corridorSize+dy+corridorSize/2), corridorSize/2-1);
}
Layer* maketext(TextLayer **tl, const char* text, Layer *parent,
                int x, int y, int w, int h, const char* font, GColor fg, GColor bg) {
  
  *tl = text_layer_create(GRect(x, y, w, h));
  text_layer_set_font(*tl, fonts_get_system_font(font));
  text_layer_set_text_color(*tl, fg);
  text_layer_set_background_color(*tl, bg);
  text_layer_set_text(*tl, text);
  text_layer_set_overflow_mode(*tl, GTextOverflowModeWordWrap);
  text_layer_set_text_alignment(*tl, GTextAlignmentCenter);
  Layer *result = text_layer_get_layer(*tl);
  layer_add_child(parent, result);
  return result;
}

static void blink(void* foo) {
  inverted = !inverted;
  text_layer_set_text_color(s_bplay_layer,inverted?GColorWhite:GColorBlack);
  text_layer_set_background_color(s_bplay_layer,inverted?GColorBlack:GColorWhite);
  if(!playing) app_timer_register(700, blink, NULL);
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect window_bounds = layer_get_bounds(window_layer);
  
  s_maze_layer = layer_create(window_bounds);
  layer_set_update_proc(s_maze_layer, maze_layer_update_callback);
  layer_add_child(window_layer, s_maze_layer);
  
  s_player_layer = layer_create(window_bounds);
  layer_set_update_proc(s_player_layer, player_layer_update_callback);
  layer_add_child(s_maze_layer, s_player_layer);
  // Create title layer
  maketext(&s_title_layer,"MAZE",window_layer,0,0,window_bounds.size.w, window_bounds.size.h,
          FONT_KEY_BITHAM_42_BOLD,GColorBlack,GColorClear);
  //create play button
  maketext(&s_bplay_layer,"play",window_layer,45,60,window_bounds.size.w-90, 40,
          FONT_KEY_GOTHIC_28_BOLD,GColorBlack,GColorWhite);
  maketext(&s_diff_layer,"medium",window_layer,45,window_bounds.size.h-20,window_bounds.size.w-90, 20,
          FONT_KEY_GOTHIC_14,GColorBlack,GColorWhite);
  blink(NULL);
}

static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_title_layer);
  text_layer_destroy(s_bplay_layer);
  layer_destroy(s_maze_layer);
  layer_destroy(s_player_layer);
}

static void updatemaze() {
  const char *names[] = {"caveman","easy","medium","hard"};
  text_layer_set_text(s_diff_layer, names[difficulty]);
  int widths[] = {0,10,18,36};
  int heights[] = {0,11,19,38};
  int sizes[] = {0,14,8,4};
  load(widths[difficulty],heights[difficulty],sizes[difficulty]);
}

static void select(ClickRecognizerRef ref, void* context) {
  if(playing) return;
  playing = TRUE;
  layer_set_hidden(text_layer_get_layer(s_title_layer),TRUE);
  layer_set_hidden(text_layer_get_layer(s_bplay_layer),TRUE);
  layer_set_hidden(text_layer_get_layer(s_diff_layer),TRUE);
  data_handler(NULL);
}
static void up(ClickRecognizerRef ref, void* context) {
 difficulty++;
  if(difficulty>3) difficulty = 3;
  else updatemaze();
}
static void down(ClickRecognizerRef ref, void* context) {
 difficulty--;
  if(difficulty<1) difficulty = 1;
  else updatemaze();
}

static void clickprovider(void *context) {
  //set our button event handler
  window_single_click_subscribe(BUTTON_ID_SELECT, select);
  window_single_click_subscribe(BUTTON_ID_UP, up);
  window_single_click_subscribe(BUTTON_ID_DOWN, down);
}


static void init() {
  srand(time(NULL));
  lightup();
  // Create maze
  mazeWidth = 18;
  mazeHeight = 19;
  corridorSize = 8;
  maze = genmaze(mazeWidth, mazeHeight);
  
  // Init Accelerometer
  accel_data_service_subscribe(10, NULL);
  accel_service_set_sampling_rate(ACCEL_SAMPLING_10HZ);
  
  // Init Player Position
  playerX = 0;
  playerY = 0; 
  

  // Create main Window
  s_main_window = window_create();
  window_set_click_config_provider(s_main_window,clickprovider);
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
  
}

static void deinit() {
  // Destroy main Window
  window_destroy(s_main_window);
  free(maze);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
