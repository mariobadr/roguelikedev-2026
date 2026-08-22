/**
 * @file tile_map.h
 */
#ifndef GINC_ROGUELIKE_TILE_MAP_H
#define GINC_ROGUELIKE_TILE_MAP_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "tile.h"

/**
 * A tile map.
 */
struct rl_tile_map
{
  /** The width of the map in tiles. */
  int width;
  /** The height of the map in tiles. */
  int height;
  /** The tiles in the map. */
  struct rl_tile* tiles;
};

/**
 * Initialize a new map with width and height.
 */
bool
rl_init_map(struct rl_tile_map* map, int width, int height);

/**
 * Free up resources used by map.
 */
void
rl_free_map(struct rl_tile_map* map);

/**
 * @return the row-major array index of (x, y) in map.
 */
static inline size_t
rl_map_index_of(struct rl_tile_map const* map, int x, int y)
{
  return y * map->width + x;
}

/**
 * @return whether the coordinates (x, y) are valid in this map
 */
bool
rl_map_contains(struct rl_tile_map const* map, int x, int y);

/**
 * @return the tile found at (x, y) in map.
 */
struct rl_tile
rl_get_tile(struct rl_tile_map const* map, int x, int y);

void
rl_set_tile(struct rl_tile_map* map, int x, int y, struct rl_tile tile);

void
rl_fill_rect(struct rl_tile_map* map,
             SDL_Rect const* rect,
             struct rl_tile tile);

#endif // GINC_ROGUELIKE_TILE_MAP_H
