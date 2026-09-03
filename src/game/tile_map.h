/**
 * @file tile_map.h
 */
#ifndef GINC_ROGUELIKE_TILE_MAP_H
#define GINC_ROGUELIKE_TILE_MAP_H

#include <SDL3/SDL_rect.h>

#include "container/grid.h"

#include "tile.h"

grid_define_as(enum rl_tile, rl_tile);

void
rl_fill_rect(grid(rl_tile) * map, SDL_Rect const* rect, enum rl_tile tile);

#endif // GINC_ROGUELIKE_TILE_MAP_H
