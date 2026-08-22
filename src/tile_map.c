#include "tile_map.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

bool
rl_init_map(struct rl_tile_map* map, int width, int height)
{
  // check whether map has already been initialized
  SDL_assert(map->tiles == NULL);

  map->tiles = SDL_calloc(width * height, sizeof(*map->tiles));
  if (map->tiles == NULL) {
    SDL_Log("SDL_calloc failed: %s", SDL_GetError());
    return false;
  }

  map->width = width;
  map->height = height;

  return true;
}

void
rl_free_map(struct rl_tile_map* map)
{
  if (map == NULL) {
    return;
  }

  SDL_free(map->tiles);
  map->tiles = NULL;

  map->width = 0;
  map->height = 0;
}

bool
rl_map_contains(struct rl_tile_map const* map, int x, int y)
{
  return x >= 0 && y >= 0 && x < map->width && y < map->height;
}

struct rl_tile
rl_get_tile(struct rl_tile_map const* map, int x, int y)
{
  SDL_assert(rl_map_contains(map, x, y));

  return map->tiles[y * map->width + x];
}

void
rl_set_tile(struct rl_tile_map* map, int x, int y, struct rl_tile tile)
{
  size_t index = rl_map_index_of(map, x, y);
  map->tiles[index] = tile;
}

void
rl_fill_rect(struct rl_tile_map* map, SDL_Rect const* rect, struct rl_tile tile)
{
  for (int y = rect->y; y < rect->y + rect->h; ++y) {
    for (int x = rect->x; x < rect->x + rect->w; ++x) {
      rl_set_tile(map, x, y, tile);
    }
  }
}
