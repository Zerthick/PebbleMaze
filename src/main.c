/*
 * main.c
 * Constructs a Window housing an output TextLayer to show data from 
 * either modes of operation of the accelerometer.
 */

#include <pebble.h>
#include "maze.h"


#define getPOS(y,x,w) ((y)*w+(x))
static Window *s_main_window;
static TextLayer *s_output_layer;
static Layer *s_maze_layer;
static Layer *s_player_layer;

static Cell *maze;
static int playerX;
static int playerY;
static int mazeWidth;
static int mazeHeight;
static int corridorSize;

static void data_handler(AccelData *data, uint32_t num_samples) {
  if (data[0].x > 20){
    playerX++;
    layer_mark_dirty(s_player_layer);
  } else if (data[0].x < -20){
    playerX--;
    layer_mark_dirty(s_player_layer);
  }
  if (data[0].y > 20){
    playerY++;
    layer_mark_dirty(s_player_layer);
  } else if (data[0].y < -20){
    playerY--;
    layer_mark_dirty(s_player_layer);
  }
}

/*static void tap_handler(AccelAxisType axis, int32_t direction) {
  switch (axis) {
  case ACCEL_AXIS_X:
    if (direction > 0) {
      text_layer_set_text(s_output_layer, "X axis positive.");
    } else {
      text_layer_set_text(s_output_layer, "X axis negative.");
    }
    break;
  case ACCEL_AXIS_Y:
    if (direction > 0) {
      text_layer_set_text(s_output_layer, "Y axis positive.");
    } else {
      text_layer_set_text(s_output_layer, "Y axis negative.");
    }
    break;
  case ACCEL_AXIS_Z:
    if (direction > 0) {
      text_layer_set_text(s_output_layer, "Z axis positive.");
    } else {
      text_layer_set_text(s_output_layer, "Z axis negative.");
    }
    break;
  }
}*/

static void maze_layer_update_callback(Layer *layer, GContext *ctx){
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
}

static void player_layer_update_callback(Layer *layer, GContext *ctx){
    graphics_draw_circle(ctx, GPoint(playerX+3, playerY+3), 2);
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
  // Create output TextLayer
  /*s_output_layer = text_layer_create(GRect(5, 0, window_bounds.size.w - 10, window_bounds.size.h));
  text_layer_set_font(s_output_layer, fonts_get_system_font(FONT_KEY_GOTHIC_24));
  text_layer_set_text(s_output_layer, "No data yet.");
  text_layer_set_overflow_mode(s_output_layer, GTextOverflowModeWordWrap);
  layer_add_child(window_layer, text_layer_get_layer(s_output_layer));*/
}

static void main_window_unload(Window *window) {
  // Destroy output TextLayer
  text_layer_destroy(s_output_layer);
  layer_destroy(s_maze_layer);
  layer_destroy(s_player_layer);
}

static void init() {
  srand(2);
  // Create maze
  mazeWidth = 18;
  mazeHeight = 18;
  corridorSize = 6;
  maze = genmaze(mazeWidth, mazeHeight);
  
  // Init Player Position
  playerX = 0;
  playerY = 0;
  
  int num_samples = 1;
  accel_data_service_subscribe(num_samples, data_handler);
  
  // Choose update rate
  accel_service_set_sampling_rate(ACCEL_SAMPLING_100HZ);
  
  // Create main Window
  s_main_window = window_create();
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });
  window_stack_push(s_main_window, true);
}

static void deinit() {
  // Destroy main Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
