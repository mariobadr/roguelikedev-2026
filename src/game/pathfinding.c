#include "pathfinding.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "tile_map.h"

bool
rl_build_dijkstra_map(grid(int) * distances,
                      struct rl_tile_map const* map,
                      SDL_Point target)
{
  SDL_assert(grid_same_shape(distances, &map->tiles));

  int const length = rl_map_height(map) * rl_map_width(map);

  if (!rl_map_contains(map, target.x, target.y) ||
      !rl_is_walkable(rl_get_tile(map, target.x, target.y))) {
    // can't reach the target?
    return false;
  }

  // TODO: this allocates the queue on *every* call. fix it?
  size_t* queue = SDL_calloc(length, sizeof(*queue));
  if (queue == NULL) {
    SDL_Log("SDL_calloc failed: %s", SDL_GetError());
    return false;
  }

  for (int i = 0; i < length; i++) {
    *grid_at_index(distances, i) = RL_INFINITE_DISTANCE;
  }

  size_t index = rl_map_index_of(map, target.x, target.y);
  *grid_at_index(distances, index) = 0;

  size_t head = 0;
  size_t tail = 0;
  queue[tail++] = index;

  while (head < tail) {
    size_t current_index = queue[head++];
    int current_x = (int)current_index % rl_map_width(map);
    int current_y = (int)current_index / rl_map_width(map);

    // for each neighbour of current
    for (size_t i = 0; i < SDL_arraysize(RL_PATH_DIRS); ++i) {
      int next_x = current_x + RL_PATH_DIRS[i].x;
      int next_y = current_y + RL_PATH_DIRS[i].y;

      // only consider walkable tiles
      if (rl_map_contains(map, next_x, next_y) &&
          rl_is_walkable(rl_get_tile(map, next_x, next_y))) {
        int new_distance = *grid_at_index(distances, current_index) + 1;

        size_t neighbour = rl_map_index_of(map, next_x, next_y);
        if (new_distance < *grid_at_index(distances, neighbour)) {
          *grid_at_index(distances, neighbour) = new_distance;
          queue[tail++] = neighbour;
        }
      }
    }
  }

  SDL_free(queue);
  return true;
}
