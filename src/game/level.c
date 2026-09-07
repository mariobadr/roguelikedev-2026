#include "level.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

static void
carve_map(grid(rl_tile) * map, struct rl_layout const* layout)
{
  enum rl_tile tile = RL_TILE_FLOOR;

  for (int i = 0; i < array_len(&layout->rooms); i++) {
    rl_fill_rect(map, array_at(&layout->rooms, i), tile);
  }

  for (int i = 0; i < array_len(&layout->corridors); i++) {
    struct rl_corridor const* corridor = array_at(&layout->corridors, i);
    for (int j = 0; j < corridor->segment_count; j++) {
      rl_fill_rect(map, &corridor->segments[j], tile);
    }
  }
}

bool
rl_alloc_level(struct rl_level* level, int depth, int width, int height)
{
  if (!grid_alloc(&level->map, width, height)) {
    SDL_Log("grid_alloc failed: %s", SDL_GetError());
    rl_free_level(level);
    return false;
  }

  if (!grid_alloc(&level->explored, width, height)) {
    SDL_Log("grid_alloc failed: %s", SDL_GetError());
    rl_free_level(level);
    return false;
  }

  level->depth = depth;

  return true;
}

void
rl_free_level(struct rl_level* level)
{
  if (level == NULL) {
    return;
  }

  rl_free_layout(&level->layout);

  grid_free(&level->explored);
  grid_free(&level->map);
}

bool
rl_gen_level(struct rl_level* level, struct rand_state* rng)
{
  int const width = grid_width(&level->map);
  int const height = grid_height(&level->map);

  // randomly generate the dungeon layout
  if (!rl_init_layout(&level->layout, width, height, rng)) {
    return false;
  }

  // update the tiles in the map based on the layout
  carve_map(&level->map, &level->layout);

  return true;
}
