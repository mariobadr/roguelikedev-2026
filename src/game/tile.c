#include "tile.h"

bool
rl_is_walkable(enum rl_tile tile)
{
  switch (tile) {
    case RL_TILE_WALL:
      return false;
    case RL_TILE_FLOOR:
    case RL_TILE_STAIRS_UP:
    case RL_TILE_STAIRS_DOWN:
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
    case RL_TILE_STAIRS_UP:
    case RL_TILE_STAIRS_DOWN:
      return true;
    default:
      break;
  }

  return false;
}

bool
rl_is_staircase(enum rl_tile tile)
{
  return tile == RL_TILE_STAIRS_UP || tile == RL_TILE_STAIRS_DOWN;
}