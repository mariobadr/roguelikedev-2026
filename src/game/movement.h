/**
 * @file movement.h
 */
#ifndef GINC_ROGUELIKE_MOVEMENT_H
#define GINC_ROGUELIKE_MOVEMENT_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "container/grid.h"

#include "game/tile.h"

/**
 * The directions of a single step (no diagonals).
 */
enum rl_step
{
  RL_STEP_RIGHT,
  RL_STEP_LEFT,
  RL_STEP_DOWN,
  RL_STEP_UP,
};

/**
 * The offsets of a single step, indexed by rl_step.
 */
static SDL_Point const RL_STEP_DIRS[] = {
  [RL_STEP_RIGHT] = { 1, 0 },
  [RL_STEP_LEFT] = { -1, 0 },
  [RL_STEP_DOWN] = { 0, 1 },
  [RL_STEP_UP] = { 0, -1 },
};

/**
 * @return whether b is a single step from a.
 */
static inline bool
rl_are_adjacent(SDL_Point a, SDL_Point b)
{
  for (size_t i = 0; i < SDL_arraysize(RL_STEP_DIRS); i++) {
    if (a.x + RL_STEP_DIRS[i].x == b.x && a.y + RL_STEP_DIRS[i].y == b.y) {
      return true;
    }
  }

  return false;
}

/**
 * @return whether an actor can walk on the tile at p.
 */
static inline bool
rl_can_walk(grid(rl_tile) const* map, SDL_Point p)
{
  return rl_is_walkable(*grid_at(map, p.x, p.y));
}

#endif // GINC_ROGUELIKE_MOVEMENT_H
