#ifndef _ENGINE_H
#define _ENGINE_H

typedef struct vector { float x; float y; } vector;

typedef struct engine {
  vector pos;
  vector dir;
  vector plane;
  vector move;
  float scale; /* scale must change as the distance to the camera plane changes */

  int* map;
  unsigned int* buf;

  int mapWidth;
  int mapHeight;

  float zbuf[1920];

  int width;
  int height;
} engine;

#ifdef ENGINE_IMPL

unsigned int engine_textures[4][4096];

static void vector_add (vector *out, vector *v1, vector *v2)
{
  out->x = v1->x + v2->x;
  out->y = v1->y + v2->y;
}

static void vector_scale (vector *out, vector *v, float s)
{
  out->x = v->x * s;
  out->y = v->y * s;
}

static void vector_rotate (vector *v, float a)
{
  float c = cos(a);
  float s = sin(a);
  float y = c * v->y - s * v->x;
  float x = c * v->x + s * v->y;

  v->x = x;
  v->y = y;
}

static void vector_init (vector *v, float x, float y)
{
  v->x = x;
  v->y = y;
}

void engine_init (engine *e, int *map, int mw, int mh, unsigned int *buf, int bw, int bh)
{
  int i;

  vector_init(&e->pos, 5.0, 9.0);
  vector_init(&e->dir, 0.0, -1.0);
  vector_init(&e->plane, 0.7, 0.0);

  e->scale = ((float)bh / (float)bw) * e->plane.x;
  e->map = map;
  e->buf = buf;

  e->width = bw;
  e->height = bh;
  e->mapWidth = mw;
  e->mapHeight = mh;

  for (i = 0; i < 1920; i++) {
    e->zbuf[i] = 1000.0;
  }
}

void engine_handle_collisions (engine *e, vector* v)
{
  /*
  int xMap = (int)floor(e->pos.x);
  int yMap = (int)floor(e->pos.y);
  */

  int xMapNext = (int)floor(e->pos.x + v->x);
  int yMapNext = (int)floor(e->pos.y + v->y);

  if (
    yMapNext >= e->mapHeight || yMapNext < 0 ||
    xMapNext >= e->mapWidth  || xMapNext < 0 ||
    e->map[yMapNext * e->mapWidth + xMapNext] > 0
  ) {
    /* if hitting top or bottom make y 0 otherwise make x 0 */
    v->y = 0;
    v->x = 0;
  }
}

void engine_move_cam_forward (engine *e, float step)
{
  vector move;

  vector_scale(&move, &e->dir, step);
  engine_handle_collisions(e, &move);

  e->pos.x += move.x;
  e->pos.y += move.y;
}

void engine_move_cam_right (engine *e, float step)
{
  vector move;

  vector_scale(&move, &e->plane, step);
  engine_handle_collisions(e, &move);

  e->pos.x += move.x;
  e->pos.y += move.y;
}

void engine_rotate_cam (engine *e, float ang)
{
  vector_rotate(&e->dir, ang);
  vector_rotate(&e->plane, ang);
}

void draw_textured_vline (engine *e, int tid, int x, int y1, int y2, int texCol, float wallDist, int side)
{
  float texStart = 0.0; 
  float texStep = 63.0 / (float)(y2 - y1);

  if (y1 < 0) {
    texStart += (texStep * (-y1));
    y1 = 0;
  }
  if (y2 >= e->height) {
    y2 = e->height - 1;
  }

  if (y1 < y2) {
    unsigned int *dst = e->buf + x + (y1 * e->width);
    int count = y2 - y1;

    e->zbuf[x] = wallDist;

    do {
      unsigned int color;
      int texY = (int)floor(texStart + 0.5) & 63;
      color = (texY % 2 == 0) ? 0xffff0000 : 0xffffffff;
      color = engine_textures[tid][(texY << 6) + texCol];
      if (side == 1) color = (color >> 1) & 8355711;
      *dst = color;
      dst += e->width;
      texStart += texStep;
    } while (count--);
  }
}

void engine_draw_walls (engine *e)
{
  int x = 0;
  float hh = (float)e->height / 2.0;
  vector ray, scaled;

  for (x = 0; x < e->width; x++) {
    float rayCam = 2 * x / (float)e->width - 1;
    float xDelta, yDelta, xSide, ySide;
    int xStep, yStep, xMap, yMap;
    int hit = 0, side;
    float wallDist, xWall;
    unsigned int color;
    int y1, y2, lineHeight;
    int texCol, texId;
    float texStart, texStep;

    /* create the ray vector */
    vector_scale(&scaled, &e->plane, rayCam);
    vector_add(&ray, &e->dir, &scaled);

    xMap = (int)e->pos.x;
    yMap = (int)e->pos.y;

    xDelta = fabs(1 / ray.x);
    yDelta = fabs(1 / ray.y);

    if (ray.x < 0) {
      xStep = -1;
      xSide = (e->pos.x - xMap) * xDelta;
    } else {
      xStep = 1;
      xSide = (xMap + 1.0 - e->pos.x) * xDelta;
    }

    if (ray.y < 0) {
      yStep = -1;
      ySide = (e->pos.y - yMap) * yDelta;
    } else {
      yStep = 1;
      ySide = (yMap + 1.0 - e->pos.y) * yDelta;
    }

    while (hit == 0) {
      if (xSide < ySide) {
        xSide += xDelta;
        xMap += xStep;
        side = 0;
      } else {
        ySide += yDelta;
        yMap += yStep;
        side = 1;
      }
      texId = e->map[yMap * e->mapWidth + xMap] - 1;
      if (xMap >= e->mapWidth || xMap < 0 || yMap >= e->mapHeight || yMap < 0) {
        texId = 1;
        hit = 1;
      } else if (texId > -1) {
        hit = 1;
      }
    }

    if (side == 0) {
      wallDist = (xMap - e->pos.x + (1 - xStep) / 2) / ray.x;
    } else {
      wallDist = (yMap - e->pos.y + (1 - yStep) / 2) / ray.y;
    }

    y1 = -0.5 / (wallDist * e->scale) * hh + hh;
    y2 =  0.5 / (wallDist * e->scale) * hh + hh;

    if (side == 0) xWall = e->pos.y + wallDist * ray.y;
    else xWall = e->pos.x + wallDist * ray.x;
    xWall -= floor(xWall);

    texCol = (int)(xWall * 64);

    if ((side == 0 && ray.x > 0) || (side == 1 && ray.y < 0)) {
      texCol = 64 - texCol - 1;
    }

    draw_textured_vline(e, texId, x, y1, y2, texCol, wallDist, side);
  }
}

void engine_draw_plane (engine *e, int offs, int stride, int texId)
{
  int x1, x2, y;
  unsigned int *tmpBuf = e->buf + offs;
  unsigned int *dst;
  float hh = (float)e->height / 2.0;
  float zPrev;
  float zInvStart = 1.0;
  float zInvEnd = 1.0 / 10000.0;
  float zInvStep = (zInvEnd - zInvStart) / hh;
  float uStep, vStep, u, v, u1, v1, u2, v2;
  vector leftMost, rightMost, scaled;
  vector textCoordLeft, textCoordRight;

  zPrev = zInvStart;

  vector_scale(&scaled, &e->plane, -1);
  vector_add(&leftMost, &e->dir, &scaled);

  vector_scale(&scaled, &e->plane, 1);
  vector_add(&rightMost, &e->dir, &scaled);

  textCoordLeft.x = (leftMost.x) + e->pos.x;
  textCoordLeft.y = (leftMost.y) + e->pos.y;

  textCoordRight.x = (rightMost.x) + e->pos.x;
  textCoordRight.y = (rightMost.y) + e->pos.y;

  for (y = 0; y < (int)hh; y++) {
    float zStep = (1.0 / zInvStart) - zPrev;

    x1 = 0;
    x2 = WIDTH;

    textCoordLeft.x += (leftMost.x * zStep);
    textCoordLeft.y += (leftMost.y * zStep);

    textCoordRight.x += (rightMost.x * zStep);
    textCoordRight.y += (rightMost.y * zStep);

    u1 = textCoordLeft.x * 64;
    v1 = textCoordLeft.y * 64;

    u2 = textCoordRight.x * 64;
    v2 = textCoordRight.y * 64;

    uStep = (u2 - u1) / e->width;
    vStep = (v2 - v1) / e->width;

    dst = tmpBuf;

    u = u1;
    v = v1;

    do {
      int ty = (int)(ceil(u)) & 63;
      int tx = (int)(ceil(v)) & 63;

      *(dst++) = engine_textures[texId][(ty << 6) + tx];

      u += uStep;
      v += vStep;
    } while (++x1 < x2);

    zPrev = 1.0 / zInvStart;
    zInvStart += zInvStep;
    tmpBuf += stride;
  }
}

void engine_draw_quad (engine *e, int startX, int endX, int startY, int endY, float dist)
{
  int x, y;
  int w = e->width, h = e->height;
  unsigned int *buf, *dst;

  /* todo: clip against the 1D z buffer */
  
  if (startY < 0) startY = 0;
  if (startX < 0) startX = 0;

  if (endY >= h) endY = h - 1;
  if (endX >= w) endX = w - 1;

  buf = e->buf + (w * startY) + startX;

  for (x = startX; x <= endX; x++) {
    dst = buf;
    if (dist < e->zbuf[x]) {
      for (y = startY; y <= endY; y++) {
        unsigned int color = 0xffffff;
        *dst = color;
        dst += w;
      }
    }
    buf++;
  }
}

void engine_draw_textured_quad (engine *e, int startX, int endX, 
  int startY, int endY, float dist, int texId)
{
  int x, y;
  int w = e->width, h = e->height;
  unsigned int color;
  float ty, tx, tyStep, txStep;
  int u, v;
  float texStartY;

  if (startX >= endX || startY >= endY)
    return;

  tx = 0;
  texStartY = 63;
  txStep = 63.0 / (endX - startX);
  tyStep = 63.0 / (endY - startY);

  if (startX < 0) {
    tx += (txStep * abs(startX));
    startX = 0;
  }

  if (startY < 0) {
    texStartY -= (tyStep * abs(startY));
    startY = 0;
  }

  if (endX >= w) endX = w - 1;
  if (endY >= h) endY = h - 1;

  for (x = startX; x <= endX; x++) {
    ty = texStartY;
    if (dist < e->zbuf[x]) {
      for (y = startY; y <= endY; y++) {
        u = (int)(ceil(ty)) & 63;
        v = (int)(ceil(tx)) & 63;

        if (y >= 0 && y < h && x >= 0 && x < w) {
          color = engine_textures[texId][(u << 6) + v];
          if (color & 0x00ffffff) e->buf[y * w + x] = color;
        }

        ty -= tyStep;
      }
    }
    tx += txStep;
  }
}

void engine_draw_sprites (engine *e)
{
  int h = e->height;
  int w = e->width;
  float invDet, transformX, transformY;
  int spriteScreenX;
  int spriteHeight, startY, endY;
  int spriteWidth, startX, endX;

  vector sprite;
  vector_init(&sprite, 2.0, 2.0);

  sprite.x -= e->pos.x;
  sprite.y -= e->pos.y;

  invDet = 1.0 / (e->plane.x * e->dir.y - e->dir.x * e->plane.y);
  transformX = (e->dir.y * sprite.x - e->dir.x * sprite.y) * invDet;
  transformY = (-e->plane.y * sprite.x + e->plane.x * sprite.y) * invDet;

  if (transformY <= 0) {
    return;
  }

  /* i don't understand this */
  spriteScreenX = (int)((w / 2) * (1 + transformX / transformY));

  spriteHeight = abs((int)(h / (transformY)));
  startY = -spriteHeight / 2 + h / 2;
  endY = spriteHeight / 2 + h / 2;

  spriteWidth = abs((int)(h / (transformY)));
  startX = -spriteWidth / 2 + spriteScreenX;
  endX = spriteWidth / 2 + spriteScreenX;

  engine_draw_textured_quad(e, startX, endX, startY, endY, transformY, 3);
}

void engine_draw (engine *e)
{
  engine_draw_plane(e, (e->width * (e->height - 1)), -e->width, 1);
  engine_draw_plane(e, 0, e->width, 1);
  engine_draw_walls(e);
  engine_draw_sprites(e);

  engine_draw_textured_quad(e, 420 - 100, 420 + 100, 320, e->height - 1, -1, 2);
}

#endif
#endif
