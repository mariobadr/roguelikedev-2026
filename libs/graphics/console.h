/**
 * @file console.h
 */
#ifndef GINC_GRAPHICS_CONSOLE_H
#define GINC_GRAPHICS_CONSOLE_H

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>

#include "container/grid.h"

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

void
gfx_draw_console_cell(SDL_Renderer* renderer,
                      struct gfx_tileset const* tileset,
                      struct gfx_console_cell const* cell,
                      SDL_FRect const* dst);

void gfx_clear_console_grid(grid(gfx_console) * console);

void
gfx_draw_console_grid(SDL_Renderer* renderer,
                      struct gfx_tileset const* tileset,
                      grid(gfx_console) const* console,
                      SDL_Rect const* region,
                      SDL_FPoint at);

#endif // GINC_GRAPHICS_CONSOLE_H
