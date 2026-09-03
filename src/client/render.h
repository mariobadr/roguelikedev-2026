/**
 * @file render.h
 */
#ifndef GINC_ROGUELIKE_RENDER_H
#define GINC_ROGUELIKE_RENDER_H

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_stdinc.h>

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;

// forward declarations
struct rl_gfx_tile;
struct rl_text;

#define GLYPH_WIDTH (6.0f)
#define GLYPH_HEIGHT (8.0f)
#define FONT_ROWS (16)
#define FONT_COLS (16)

void
rl_fill_tile(SDL_Renderer* renderer, SDL_FColor colour, int col, int row);

void
rl_draw_tile(SDL_Renderer* renderer,
             SDL_Texture* font,
             struct rl_gfx_tile const* tile,
             int col,
             int row);

void
rl_draw_string(SDL_Renderer* renderer,
               SDL_Texture* font,
               char const* text,
               SDL_FColor fg,
               SDL_FColor bg,
               int col,
               int row);

void
rl_draw_text(SDL_Renderer* renderer,
             SDL_Texture* font,
             struct rl_text const* text,
             int col,
             int row);

#endif // GINC_ROGUELIKE_RENDER_H
