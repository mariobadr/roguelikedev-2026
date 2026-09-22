#include "dijkstra.h"

#include <SDL3/SDL_assert.h>

bool
sptl_alloc_dijkstra_map(struct sptl_dijkstra_map* map, int width, int height)
{
  if (!grid_alloc(&map->distances, width, height)) {
    return false;
  }

  if (!array_alloc(&map->queue, grid_count(&map->distances))) {
    grid_free(&map->distances);
    return false;
  }

  return true;
}

void
sptl_free_dijkstra_map(struct sptl_dijkstra_map* map)
{
  grid_free(&map->distances);
  array_free(&map->queue);
}

void
sptl_compute_dijkstra_map(struct sptl_dijkstra_map* map,
                          SDL_Point const* goals,
                          int goal_count,
                          struct sptl_movement const* movement,
                          void* context)
{
  grid(int)* distances = &map->distances;
  array(int)* queue = &map->queue;

  for (size_t i = 0; i < grid_count(distances); ++i) {
    *grid_at_index(distances, i) = SPTL_UNREACHABLE;
  }

  for (int i = 0; i < goal_count; ++i) {
    SDL_assert(grid_contains(distances, goals[i].x, goals[i].y));

    int* distance = grid_at(distances, goals[i].x, goals[i].y);
    if (*distance == 0) {
      continue;
    }

    *distance = 0;
    *array_push(queue) = (int)grid_index_of(distances, goals[i].x, goals[i].y);
  }

  for (size_t head = 0; head < array_len(queue); ++head) {
    int const index = *array_at(queue, head);
    SDL_Point const current = { index % grid_width(distances),
                                index / grid_width(distances) };
    int const next_distance = *grid_at_index(distances, index) + 1;

    for (int i = 0; i < movement->dir_count; ++i) {
      SDL_Point const neighbour = { current.x + movement->dirs[i].x,
                                    current.y + movement->dirs[i].y };

      if (!grid_contains(distances, neighbour.x, neighbour.y)) {
        continue;
      }

      if (!movement->can_step(context, neighbour, current)) {
        continue;
      }

      int* distance = grid_at(distances, neighbour.x, neighbour.y);
      if (next_distance < *distance) {
        *distance = next_distance;
        *array_push(queue) =
          (int)grid_index_of(distances, neighbour.x, neighbour.y);
      }
    }
  }

  array_clear(queue);
}
