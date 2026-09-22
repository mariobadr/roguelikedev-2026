/**
 * @file dijkstra.h
 */
#ifndef GINC_SPATIAL_DIJKSTRA_H
#define GINC_SPATIAL_DIJKSTRA_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "container/array.h"
#include "container/grid.h"

/** The distance of a tile from which no goal can be reached. */
#define SPTL_UNREACHABLE SDL_MAX_SINT32

/**
 * The search runs outwards from the goals, so it is called with the neighbour
 * as from and the current tile as to, in the direction actors walk.
 *
 * @return whether an actor can step from one tile to an adjacent one.
 */
typedef bool (*sptl_step_fn)(void* context, SDL_Point from, SDL_Point to);

/**
 * The steps an actor can take from a tile.
 */
struct sptl_movement
{
  /** The offsets of a single step. */
  SDL_Point const* dirs;
  /** The number of offsets in dirs. */
  int dir_count;
  /** Whether a single step is allowed. */
  sptl_step_fn can_step;
};

/**
 * The number of steps from each tile to its nearest goal.
 *
 * @invariant the capacity of queue is the number of tiles in distances.
 */
struct sptl_dijkstra_map
{
  /** The number of steps to the nearest goal, or SPTL_UNREACHABLE. */
  grid(int) distances;
  /** Scratch space for the search. */
  array(int) queue;
};

/**
 * @return whether allocation succeeded.
 */
bool
sptl_alloc_dijkstra_map(struct sptl_dijkstra_map* map, int width, int height);

/**
 * Free the map's storage.
 */
void
sptl_free_dijkstra_map(struct sptl_dijkstra_map* map);

/**
 * Compute the number of steps from every tile to its nearest goal.
 *
 * @param goals must be inside the map.
 */
void
sptl_compute_dijkstra_map(struct sptl_dijkstra_map* map,
                          SDL_Point const* goals,
                          int goal_count,
                          struct sptl_movement const* movement,
                          void* context);

#endif // GINC_SPATIAL_DIJKSTRA_H
