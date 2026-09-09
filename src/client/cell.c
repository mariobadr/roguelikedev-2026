#include "cell.h"

int
rl_cell_width(void)
{
  return 6;
}

int
rl_cell_height(void)
{
  return 8;
}

SDL_FPoint
rl_cell_point_to_pixels(SDL_Point const* point)
{
  SDL_FPoint pixels = { 0 };
  pixels.x = (float)(point->x * rl_cell_width());
  pixels.y = (float)(point->y * rl_cell_height());

  return pixels;
}

SDL_FRect
rl_cell_rect_to_pixels(SDL_Rect const* rect)
{
  SDL_FRect pixels = { 0 };
  pixels.x = (float)(rect->x * rl_cell_width());
  pixels.y = (float)(rect->y * rl_cell_height());
  pixels.w = (float)(rect->w * rl_cell_width());
  pixels.h = (float)(rect->h * rl_cell_height());

  return pixels;
}
