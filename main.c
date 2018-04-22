#include <stdio.h>

#define TEXT_IMPL
#include "text.h"

#define WIDTH 640
#define HEIGHT 480

#define WINDOW_IMPL
#include "window.h"

#define ENGINE_IMPL
#include "engine.h"

#define map_width 10
#define map_height 10

int map_data[] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 2, 2, 2, 2, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 2, 0, 0, 0,
  0, 0, 0, 1, 0, 0, 2, 0, 0, 0,
  0, 0, 0, 1, 1, 0, 2, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* temporary */
static void generate_textures ()
{ 
  int x, y, size = 64;

  for (x = 0; x < size; x++) {
    for (y = 0; y < size; y++) {
      int xycolor = y * 128 / size + x * 128 / size;

      engine_textures[0][size * y + x] = 65536 * 254 * (x != y && x != size - y);
      engine_textures[1][size * y + x] = xycolor + 256 * xycolor + 65536 * xycolor;
      engine_textures[2][size * y + x] = 256 * xycolor + 65536 * xycolor;
    }
  }
}

int main (int argc, char **argv)
{
  unsigned int start;
  unsigned int frame;
  unsigned int* buf;
  int bufsize;
  engine e;

  Window window;

  bufsize = WIDTH * HEIGHT * sizeof(unsigned int);

  buf = malloc(bufsize);

  if (!buf) {
    return 1;
  }

  engine_init(&e, map_data, map_width, map_height, buf, WIDTH, HEIGHT);
  generate_textures();
  windowInit(&window, "Game");

  start = windowGetTicks();

  while (!window.quit) {
    char debug_string[100];

    start = windowGetTicks();

    if (window.keys.left)  engine_rotate_cam(&e, 0.02);
    if (window.keys.right) engine_rotate_cam(&e, -0.02);
    if (window.keys.a)     engine_move_cam_right(&e, -0.05);
    if (window.keys.d)     engine_move_cam_right(&e, 0.05);
    if (window.keys.w)     engine_move_cam_forward(&e, 0.05);
    if (window.keys.s)     engine_move_cam_forward(&e, -0.05);

    engine_draw(&e);

    /* actual frame time */
    frame = windowGetTicks() - start;

    /* render text */
    sprintf(debug_string, "resolution: %dx%d", WIDTH, HEIGHT);
    draw_text(buf, WIDTH, HEIGHT, (const char *)debug_string, strlen(debug_string), 10, 10);
    sprintf(debug_string, "frame: %dms %dfps", frame, frame == 0 ? 1000 : (1000 / frame));
    draw_text(buf, WIDTH, HEIGHT, (const char *)debug_string, strlen(debug_string), 10, 20);

    windowUpdate(&window, buf);
    memset(buf, 0, bufsize);
  }

  windowDestroy(&window);
  free(buf);
  return 0;
}

