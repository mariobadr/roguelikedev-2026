/**
 * @file tileset.h
 */
#ifndef GINC_GRAPHICS_TILESET_H
#define GINC_GRAPHICS_TILESET_H

#include <SDL3/SDL_rect.h>

// external forward declarations
typedef struct SDL_Texture SDL_Texture;

/**
 * A texture holding a grid of equally sized tiles.
 */
struct gfx_tileset
{
  /** Texture holding the tiles. */
  SDL_Texture* texture;
  /** The width of one tile, in pixels. */
  int tile_width;
  /** The height of one tile, in pixels. */
  int tile_height;
  /** The number of tiles per row. */
  int columns;
};

/**
 * @return the source rectangle of the tile at index in tileset.
 */
SDL_FRect
gfx_tileset_src(struct gfx_tileset const* tileset, int index);

/**
 * @return a destination for a tile from tileset.
 */
SDL_FRect
gfx_tileset_dst(struct gfx_tileset const* tileset, SDL_FPoint at, int scale);

#endif // GINC_GRAPHICS_TILESET_H
