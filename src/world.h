/**
 * @file world.h
 */
#ifndef GINC_ROGUELIKE_WORLD_H
#define GINC_ROGUELIKE_WORLD_H

#include <SDL3/SDL_stdinc.h>

/**
 * A world tile.
 */
struct rl_world_tile
{
  bool walkable;
};

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
  struct rl_world_tile* tiles;
};

/**
 * Initialize a new map with width and height.
 */
bool
rl_init_map(struct rl_world_map* map, int width, int height);

/**
 * Assign the tiles in map to something interesting.
 */
void
rl_generate_map(struct rl_world_map* map);

#endif // GINC_ROGUELIKE_WORLD_H
