/**
 * @file tile.h
 */
#ifndef GINC_ROGUELIKE_TILE_H
#define GINC_ROGUELIKE_TILE_H

#include <SDL3/SDL_stdinc.h>

#include "container/grid.h"

/**
 * The different tile types.
 */
enum rl_tile
{
  RL_TILE_WALL,  //< A wall tile
  RL_TILE_FLOOR, //< A floor tile
};

/**
 * @return whether an actor can walk on this tile
 */
bool
rl_is_walkable(enum rl_tile tile);

/**
 * @return whether an actor can see through this tile
 */
bool
rl_is_transparent(enum rl_tile tile);

/**
 * A tile map as a 2D grid of tiles.
 */
grid_define_as(enum rl_tile, rl_tile);

#endif // GINC_ROGUELIKE_TILE_H
