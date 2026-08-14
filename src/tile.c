#include "tile.h"

bool
rl_is_walkable(struct rl_tile tile)
{
  switch (tile.type) {
    case RL_TILE_WALL:
      return false;
    case RL_TILE_FLOOR:
      return true;
    default:
      break;
  }

  return false;
}
