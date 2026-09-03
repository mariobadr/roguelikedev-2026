/**
 * @file tile_map.h
 */
#ifndef GINC_ROGUELIKE_TILE_MAP_H
#define GINC_ROGUELIKE_TILE_MAP_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "container/grid.h"

#include "tile.h"

grid_define_as(enum rl_tile, rl_tile);

/**
 * A tile map.
 */
struct rl_tile_map
{
  /** The tiles in the map. */
  grid(rl_tile) tiles;
};

/**
 * @return the width of the map in tiles.
 */
static inline int
rl_map_width(struct rl_tile_map const* map)
{
  return grid_width(&map->tiles);
}

/**
 * @return the height of the map in tiles.
 */
static inline int
rl_map_height(struct rl_tile_map const* map)
{
  return grid_height(&map->tiles);
}

/**
 * Initialize a new map with width and height.
 */
bool
rl_init_map(struct rl_tile_map* map, int width, int height);

/**
 * Free up resources used by map.
 */
void
rl_free_map(struct rl_tile_map* map);

/**
 * @return the row-major array index of (x, y) in map.
 */
static inline size_t
rl_map_index_of(struct rl_tile_map const* map, int x, int y)
{
  return grid_index_of(&map->tiles, x, y);
}

/**
 * @return whether the coordinates (x, y) are valid in this map
 */
bool
rl_map_contains(struct rl_tile_map const* map, int x, int y);

/**
 * @return the tile found at (x, y) in map.
 */
enum rl_tile
rl_get_tile(struct rl_tile_map const* map, int x, int y);

void
rl_set_tile(struct rl_tile_map* map, int x, int y, enum rl_tile tile);

void
rl_fill_rect(struct rl_tile_map* map,
             SDL_Rect const* rect,
             enum rl_tile tile);

#endif // GINC_ROGUELIKE_TILE_MAP_H
