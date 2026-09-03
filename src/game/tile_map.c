#include "tile_map.h"

void
rl_fill_rect(grid(rl_tile) * map, SDL_Rect const* rect, enum rl_tile tile)
{
  for (int y = rect->y; y < rect->y + rect->h; ++y) {
    for (int x = rect->x; x < rect->x + rect->w; ++x) {
      *grid_at(map, x, y) = tile;
    }
  }
}
