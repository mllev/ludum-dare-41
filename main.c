#include <stdio.h>

#define TEXT_IMPL
#include "text.h"

#define WIDTH 640
#define HEIGHT 480

#define BMPREAD_IMPL
#include "bmpread.h"

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
  0, 0, 0, 1, 1, 1, 1, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 0, 0, 0,
  0, 0, 0, 1, 0, 0, 1, 0, 0, 0,
  0, 0, 0, 1, 1, 0, 1, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

/* temporary */
static void generate_textures ()
{ 
  int x, y, w = 64, h = 64;
  bmpread_t bmp1, bmp2, bmp3, bmp4;

  bmpread("./assets/eagle.bmp", 0, &bmp1);
  bmpread("./assets/greystone.bmp", 0, &bmp2);
  bmpread("./assets/gun.bmp", 0, &bmp3);
  bmpread("./assets/barrel.bmp", 0, &bmp4);

#define rgba(r, g, b, a) ((a << 24) | (r << 16) | (g << 8) | b)
#define rgb(r, g, b) rgba((r), (g), (b), 255)
  
  for (x = 0; x < w; x++) {
    for (y = 0; y < h; y++) {
      int idx = w * y + x;
      engine_textures[0][idx] = rgb(bmp1.rgb_data[idx*3], bmp1.rgb_data[idx*3+1], bmp1.rgb_data[idx*3+2]); 
      engine_textures[1][idx] = rgb(bmp2.rgb_data[idx*3], bmp2.rgb_data[idx*3+1], bmp2.rgb_data[idx*3+2]); 
      engine_textures[2][idx] = rgb(bmp3.rgb_data[idx*3], bmp3.rgb_data[idx*3+1], bmp3.rgb_data[idx*3+2]);     
      engine_textures[3][idx] = rgb(bmp4.rgb_data[idx*3], bmp4.rgb_data[idx*3+1], bmp4.rgb_data[idx*3+2]);     
    }
  }

#undef rgba
#undef rgb
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

    if (window.keys.left)  engine_rotate_cam(&e, 0.04);
    if (window.keys.right) engine_rotate_cam(&e, -0.04);
    if (window.keys.a)     engine_move_cam_right(&e, -0.1);
    if (window.keys.d)     engine_move_cam_right(&e, 0.1);
    if (window.keys.w)     engine_move_cam_forward(&e, 0.1);
    if (window.keys.s)     engine_move_cam_forward(&e, -0.1);

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

