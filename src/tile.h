/**
 * @file tile.h
 */
#ifndef GINC_ROGUELIKE_TILE_H
#define GINC_ROGUELIKE_TILE_H

#include <SDL3/SDL_stdinc.h>

/**
 * The different tile types.
 */
enum rl_tile_type
{
  RL_TILE_WALL,  //< A wall tile
  RL_TILE_FLOOR, //< A floor tile
};

/**
 * A tile.
 */
struct rl_tile
{
  /** The type of the tile. */
  enum rl_tile_type type;
};

/**
 * @return whether an entity can walk on this tile
 */
bool
rl_is_walkable(struct rl_tile tile);

/**
 * @return whether an entity can see through this tile
 */
bool
rl_is_transparent(struct rl_tile tile);

#endif // GINC_ROGUELIKE_TILE_H
