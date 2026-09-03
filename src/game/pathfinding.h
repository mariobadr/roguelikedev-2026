/**
 * @file pathfinding.h
 */
#ifndef GINC_ROGUELIKE_PATHFINDING_H
#define GINC_ROGUELIKE_PATHFINDING_H

#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_rect.h>

#include "container/array.h"

// forward declarations
struct rl_tile_map;

#define RL_INFINITE_DISTANCE SDL_MAX_SINT32

/**
 * Possible directions in a path (no diagonals).
 */
static SDL_Point const RL_PATH_DIRS[] = { { 1, 0 },
                                          { -1, 0 },
                                          { 0, 1 },
                                          { 0, -1 } };

bool
rl_build_dijkstra_map(array(int) * distances,
                      struct rl_tile_map const* map,
                      SDL_Point target);

#endif // GINC_ROGUELIKE_PATHFINDING_H
