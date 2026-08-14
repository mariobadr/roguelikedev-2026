/**
 * @file map.h
 */
#ifndef GINC_ROGUELIKE_MAP_H
#define GINC_ROGUELIKE_MAP_H

#include <SDL3/SDL_stdinc.h>

#include "tile.h"

/**
 * A world map.
 */
struct rl_world_map
{
  /** The width of the map in tiles. */
  int width;
  /** The height of the map in tiles. */
  int height;
  /** The world tiles in the map. */
  struct rl_tile* tiles;
};

/**
 * Initialize a new map with width and height.
 */
bool
rl_init_map(struct rl_world_map* map, int width, int height);

/**
 * Get the tile found at (x, y) in map.
 */
struct rl_tile
rl_get_tile(struct rl_world_map const* map, int x, int y);

#endif // GINC_ROGUELIKE_MAP_H
