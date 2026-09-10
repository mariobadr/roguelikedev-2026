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
struct rl_font;
struct rl_gfx_tile;
struct rl_text;

void
rl_draw_tile(SDL_Renderer* renderer,
             struct rl_font const* font,
             struct rl_gfx_tile const* tile,
             SDL_FPoint at);

void
rl_draw_string(SDL_Renderer* renderer,
               struct rl_font const* font,
               char const* text,
               SDL_FColor fg,
               SDL_FColor bg,
               SDL_FPoint at);

void
rl_draw_text(SDL_Renderer* renderer,
             struct rl_font const* font,
             struct rl_text const* text,
             SDL_FColor fg,
             SDL_FColor bg,
             SDL_FPoint at);

#endif // GINC_ROGUELIKE_RENDER_H
