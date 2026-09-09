#include "anchor.h"

SDL_FRect
ui_resolve(struct ui_position p, SDL_FRect const* bounds, float w, float h)
{
  float x = bounds->x;
  float y = bounds->y;

  if (p.anchor & UI_ANCHOR_RIGHT) {
    x = bounds->x + bounds->w - w;
  } else if (p.anchor & UI_ANCHOR_CENTRE_X) {
    x = bounds->x + (bounds->w - w) * 0.5f;
  }

  if (p.anchor & UI_ANCHOR_BOTTOM) {
    y = bounds->y + bounds->h - h;
  } else if (p.anchor & UI_ANCHOR_CENTRE_Y) {
    y = bounds->y + (bounds->h - h) * 0.5f;
  }

  SDL_FRect rect = { x + p.offset.x, y + p.offset.y, w, h };
  return rect;
}
