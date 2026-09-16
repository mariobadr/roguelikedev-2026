/**
 * @file render.h
 */
#ifndef GINC_ROGUELIKE_RENDER_H
#define GINC_ROGUELIKE_RENDER_H

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct gfx_tileset;
struct rl_text;

void
rl_draw_string(SDL_Renderer* renderer,
               struct gfx_tileset const* font,
               char const* text,
               SDL_FColor fg,
               SDL_FColor bg,
               SDL_FPoint at);

void
rl_draw_text(SDL_Renderer* renderer,
             struct gfx_tileset const* font,
             struct rl_text const* text,
             SDL_FColor fg,
             SDL_FColor bg,
             SDL_FPoint at);

#endif // GINC_ROGUELIKE_RENDER_H
