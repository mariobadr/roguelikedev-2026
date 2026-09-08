/**
 * @file level.h
 */
#ifndef GINC_ROGUELIKE_LEVEL_H
#define GINC_ROGUELIKE_LEVEL_H

#include <SDL3/SDL_stdinc.h>

#include "procgen/layout.h"

#include "game/tile.h"

struct rl_level
{
  /** How deep this level is in the overall dungeon. */
  int depth;
  /** The layout of the level (currently only one level). */
  struct rl_layout layout;
  /** A map of the current level. */
  grid(rl_tile) map;
  /** Cells the player has seen before */
  grid(boolean) explored;
};

bool
rl_alloc_level(struct rl_level* level, int depth, int width, int height);

void
rl_free_level(struct rl_level* level);

#endif // GINC_ROGUELIKE_LEVEL_H
