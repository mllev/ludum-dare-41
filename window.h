#ifndef _WINDOW_H
#define _WINDOW_H

#include <SDL2/SDL.h>

#define ASPECT_RATIO ((float)WIDTH / (float)HEIGHT)

#define WINDOW_WIDTH 960
#define WINDOW_HEIGHT ((int)((float)WINDOW_WIDTH / ASPECT_RATIO))

typedef struct _window Window;

void windowInit(Window*, const char*);
void windowUpdate(Window*, unsigned*);
void windowDestroy(Window*);
unsigned int windowGetTicks();

#ifdef WINDOW_IMPL

struct _window {
  struct {
    int a, w, s, d;
    int up, down, left, right;
    int _1, _2;
  } keys;

  struct {
    int x, y;
  } mouse;

  int quit;

  SDL_Event event;
  SDL_Window* window;
  SDL_Renderer* renderer;
  SDL_Texture* texture;
};

void windowInit (Window *w, const char *name)
{
  int flags = SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC;
  int pflags = SDL_PIXELFORMAT_ARGB8888;
  int tflags = SDL_TEXTUREACCESS_STREAMING;

  SDL_Init(SDL_INIT_VIDEO);
  
  w->window = SDL_CreateWindow(name, 10, 10, WIDTH, HEIGHT, 0);

  SDL_SetWindowSize(w->window, WINDOW_WIDTH, WINDOW_HEIGHT);

  w->renderer = SDL_CreateRenderer(w->window, -1, flags);
  w->texture = SDL_CreateTexture(w->renderer, pflags, tflags, WIDTH, HEIGHT);
  w->quit = 0;
  w->keys.a = 0; w->keys.s = 0; w->keys.d = 0; w->keys.w = 0;
  w->keys.up = 0; w->keys.down = 0; w->keys.left = 0; w->keys.right = 0;
  w->keys._1 = 0; w->keys._2 = 0;
  w->mouse.x = 0;
  w->mouse.y = 0;
}

void windowUpdate (Window *w, unsigned *buf)
{
  int down = 1;
  SDL_Delay(10);
  SDL_UpdateTexture(w->texture, NULL, buf, WIDTH * sizeof(unsigned));
  SDL_RenderClear(w->renderer);
  SDL_RenderCopy(w->renderer, w->texture, NULL, NULL);
  SDL_RenderPresent(w->renderer);

  while (SDL_PollEvent(&(w->event))) {
    switch (w->event.type) {
      case SDL_QUIT:
        w->quit = 1;
        break;
      case SDL_KEYUP: down = 0;
      case SDL_KEYDOWN:
        switch (w->event.key.keysym.sym) {
          case SDLK_a:     w->keys.a     = down; break;
          case SDLK_d:     w->keys.d     = down; break;
          case SDLK_w:     w->keys.w     = down; break;
          case SDLK_s:     w->keys.s     = down; break;
          case SDLK_1:     w->keys._1    = down; break;
          case SDLK_2:     w->keys._2    = down; break;
          case SDLK_UP:    w->keys.up    = down; break;
          case SDLK_DOWN:  w->keys.down  = down; break;
          case SDLK_LEFT:  w->keys.left  = down; break;
          case SDLK_RIGHT: w->keys.right = down; break;
          default: break;
        }
        break;
      default: break;
    }
  }
}

unsigned int windowGetTicks ()
{
  return SDL_GetTicks();
}

void windowDestroy (Window *w)
{
  SDL_DestroyTexture(w->texture);
  SDL_DestroyRenderer(w->renderer);
  SDL_DestroyWindow(w->window);
  SDL_Quit();
}

#endif
#endif
