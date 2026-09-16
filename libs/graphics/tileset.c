#include "tileset.h"

SDL_FRect
gfx_tileset_src(struct gfx_tileset const* tileset, int index)
{
  SDL_FRect src = { 0 };
  src.x = (index % tileset->columns) * (float)tileset->tile_width;
  src.y = (index / tileset->columns) * (float)tileset->tile_height;
  src.w = (float)tileset->tile_width;
  src.h = (float)tileset->tile_height;

  return src;
}

SDL_FRect
gfx_tileset_dst(struct gfx_tileset const* tileset, SDL_FPoint at, int scale)
{
  SDL_FRect dst = { 0 };
  dst.x = at.x;
  dst.y = at.y;
  dst.w = (float)tileset->tile_width * scale;
  dst.h = (float)tileset->tile_height * scale;

  return dst;
}
