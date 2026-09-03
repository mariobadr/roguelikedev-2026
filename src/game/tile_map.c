#include "tile_map.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

bool
rl_init_map(struct rl_tile_map* map, int width, int height)
{
  // check whether map has already been initialized
  SDL_assert(map->tiles.data == NULL);

  if (!grid_alloc(&map->tiles, width, height)) {
    SDL_Log("grid_alloc failed: %s", SDL_GetError());
    return false;
  }

  return true;
}

void
rl_free_map(struct rl_tile_map* map)
{
  if (map == NULL) {
    return;
  }

  grid_free(&map->tiles);
}

bool
rl_map_contains(struct rl_tile_map const* map, int x, int y)
{
  return grid_contains(&map->tiles, x, y);
}

enum rl_tile
rl_get_tile(struct rl_tile_map const* map, int x, int y)
{
  SDL_assert(rl_map_contains(map, x, y));

  return *grid_at(&map->tiles, x, y);
}

void
rl_set_tile(struct rl_tile_map* map, int x, int y, enum rl_tile tile)
{
  *grid_at(&map->tiles, x, y) = tile;
}

void
rl_fill_rect(struct rl_tile_map* map, SDL_Rect const* rect, enum rl_tile tile)
{
  for (int y = rect->y; y < rect->y + rect->h; ++y) {
    for (int x = rect->x; x < rect->x + rect->w; ++x) {
      rl_set_tile(map, x, y, tile);
    }
  }
}
