#include "tile.h"

bool
rl_is_walkable(enum rl_tile tile)
{
  switch (tile) {
    case RL_TILE_WALL:
      return false;
    case RL_TILE_FLOOR:
      return true;
    default:
      break;
  }

  return false;
}

bool
rl_is_transparent(enum rl_tile tile)
{
  switch (tile) {
    case RL_TILE_WALL:
      return false;
    case RL_TILE_FLOOR:
      return true;
    default:
      break;
  }

  return false;
}