#include "level.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

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

