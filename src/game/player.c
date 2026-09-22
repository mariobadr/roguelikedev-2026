#include "player.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#define FOV_RADIUS 8

bool
rl_alloc_player(struct rl_player* player, int width, int height)
{
  if (!sptl_alloc_dijkstra_map(&player->dijkstra, width, height)) {
    SDL_Log("sptl_alloc_dijkstra_map failed: %s", SDL_GetError());
    return false;
  }

  if (!rl_alloc_fov(&player->fov, width, height, FOV_RADIUS)) {
    return false;
  }

  return true;
}

void
rl_free_player(struct rl_player* player)
{
  if (player == NULL) {
    return;
  }

  rl_free_fov(&player->fov);
  sptl_free_dijkstra_map(&player->dijkstra);
}
