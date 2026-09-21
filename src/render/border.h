/**
 * @file border.h
 */
#ifndef GINC_ROGUELIKE_RENDER_BORDER_H
#define GINC_ROGUELIKE_RENDER_BORDER_H

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>

#include "cp437/border.h"

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct gfx_tileset;

/**
 * Draw a CP437 box directly.
 */
void
rl_draw_cp437_box(SDL_Renderer* renderer,
                  struct gfx_tileset const* font,
                  SDL_Rect const* box,
                  struct cp437_border_style const* style,
                  SDL_FColor fg,
                  SDL_FColor bg,
                  SDL_FPoint origin);

/**
 * Draw a CP437 border grid.
 */
void
rl_draw_cp437_borders(SDL_Renderer* renderer,
                      struct gfx_tileset const* font,
                      grid(cp437_border) const* borders,
                      SDL_FColor fg,
                      SDL_FColor bg,
                      SDL_FPoint origin);

#endif // GINC_ROGUELIKE_RENDER_BORDER_H
