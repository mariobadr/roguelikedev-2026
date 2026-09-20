/**
 * @file console.h
 */
#ifndef GINC_GRAPHICS_CONSOLE_H
#define GINC_GRAPHICS_CONSOLE_H

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>

#include "container/grid.h"
#include "core/string.h"

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;

// forward declarations
struct gfx_tileset;

/**
 * A cell in a console/terminal.
 */
struct gfx_console_cell
{
  /** Index into a tileset. */
  int index;
  /** Foreground tint applied to the cell. */
  SDL_FColor fg;
  /** Background colour of the cell. */
  SDL_FColor bg;
};

/**
 * A grid of cells.
 */
grid_define_as(struct gfx_console_cell, gfx_console);

/**
 * Draw cell into dst, skipping a transparent background or foreground.
 */
void
gfx_draw_console_cell(SDL_Renderer* renderer,
                      struct gfx_tileset const* tileset,
                      struct gfx_console_cell const* cell,
                      SDL_FRect const* dst);

/**
 * Reset every cell to draw nothing.
 */
void gfx_clear_console_grid(grid(gfx_console) * console);

/**
 * Draw console from the screen position at.
 *
 * @param region the cells to draw, or NULL for all.
 */
void
gfx_draw_console_grid(SDL_Renderer* renderer,
                      struct gfx_tileset const* tileset,
                      grid(gfx_console) const* console,
                      SDL_Rect const* region,
                      SDL_FPoint at);

/**
 * Draw text as a row of glyphs from the screen position at.
 */
void
gfx_print_console(SDL_Renderer* renderer,
                  struct gfx_tileset const* font,
                  struct str_view text,
                  SDL_FColor fg,
                  SDL_FColor bg,
                  SDL_FPoint at);

#endif // GINC_GRAPHICS_CONSOLE_H
