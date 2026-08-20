/**
 * @file map.h
 */
#ifndef GINC_ROGUELIKE_MAP_H
#define GINC_ROGUELIKE_MAP_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "tile.h"

// forward declarations
struct rand_state;

/**
 * A map.
 */
struct rl_map
{
  /** The width of the map in tiles. */
  int width;
  /** The height of the map in tiles. */
  int height;
  /** The tiles in the map. */
  struct rl_tile* tiles;
  /** The rooms in the map. */
  SDL_Rect* rooms;
  /** The number of rooms in the map. */
  int room_count;
};

/**
 * Initialize a new map with width and height.
 */
bool
rl_init_map(struct rl_map* map, int width, int height, struct rand_state* rng);

/**
 * Free up resources used by map.
 */
void
rl_free_map(struct rl_map* map);

/**
 * @return the row-major array index of (x, y) in map.
 */
static inline size_t
rl_map_index_of(struct rl_map const* map, int x, int y)
{
  return (size_t)y * (size_t)map->width + (size_t)x;
}

/**
 * @return whether the coordinates (x, y) are valid in this map
 */
bool
rl_map_contains(struct rl_map const* map, int x, int y);

/**
 * @return the tile found at (x, y) in map.
 */
struct rl_tile
rl_get_tile(struct rl_map const* map, int x, int y);

#endif // GINC_ROGUELIKE_MAP_H
