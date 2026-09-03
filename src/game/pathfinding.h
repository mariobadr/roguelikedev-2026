/**
 * @file pathfinding.h
 */
#ifndef GINC_ROGUELIKE_PATHFINDING_H
#define GINC_ROGUELIKE_PATHFINDING_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "container/grid.h"

#include "tile_map.h"

#define RL_INFINITE_DISTANCE SDL_MAX_SINT32

/**
 * Possible directions in a path (no diagonals).
 */
static SDL_Point const RL_PATH_DIRS[] = { { 1, 0 },
                                          { -1, 0 },
                                          { 0, 1 },
                                          { 0, -1 } };

bool
rl_build_dijkstra_map(grid(int) * distances,
                      grid(rl_tile) const* map,
                      SDL_Point target);

#endif // GINC_ROGUELIKE_PATHFINDING_H
