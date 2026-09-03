#include "pathfinding.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "tile_map.h"

bool
rl_build_dijkstra_map(array(int) * distances,
                      struct rl_tile_map const* map,
                      SDL_Point target)
{
  int const length = map->height * map->width;

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
    *array_at(distances, i) = RL_INFINITE_DISTANCE;
  }

  size_t index = rl_map_index_of(map, target.x, target.y);
  *array_at(distances, index) = 0;

  size_t head = 0;
  size_t tail = 0;
  queue[tail++] = index;

  while (head < tail) {
    size_t current_index = queue[head++];
    int current_x = (int)current_index % map->width;
    int current_y = (int)current_index / map->width;

    // for each neighbour of current
    for (size_t i = 0; i < SDL_arraysize(RL_PATH_DIRS); ++i) {
      int next_x = current_x + RL_PATH_DIRS[i].x;
      int next_y = current_y + RL_PATH_DIRS[i].y;

      // only consider walkable tiles
      if (rl_map_contains(map, next_x, next_y) &&
          rl_is_walkable(rl_get_tile(map, next_x, next_y))) {
        int new_distance = *array_at(distances, current_index) + 1;

        size_t neighbour = rl_map_index_of(map, next_x, next_y);
        if (new_distance < *array_at(distances, neighbour)) {
          *array_at(distances, neighbour) = new_distance;
          queue[tail++] = neighbour;
        }
      }
    }
  }

  SDL_free(queue);
  return true;
}
