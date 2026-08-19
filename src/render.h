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

#define GLYPH_WIDTH (6.0f)
#define GLYPH_HEIGHT (8.0f)
#define FONT_ROWS (16)
#define FONT_COLS (16)

static const SDL_FColor RL_COLOUR_NONE = { 0.0f, 0.0f, 0.0f, 0.0f };
static const SDL_FColor RL_COLOUR_AMBIENT = { 0.06f, 0.06f, 0.06f, 1.0f };
static const SDL_FColor RL_COLOUR_BLACK = { 0.0f, 0.0f, 0.0f, 1.0f };
static const SDL_FColor RL_COLOUR_WHITE = { 1.0f, 1.0f, 1.0f, 1.0f };
static const SDL_FColor RL_COLOUR_LGRAY = { 0.8f, 0.8f, 0.9f, 1.0f };
static const SDL_FColor RL_COLOUR_DGRAY = { 0.25f, 0.25f, 0.35f, 1.0f };

/**
 * A graphical tile.
 */
struct rl_gfx_tile
{
  /** Glyph index into the font. */
  Uint8 glyph;
  /** Foreground tint applied to the glyph. */
  SDL_FColor fg;
  /** Background color of the tile. */
  SDL_FColor bg;
};

/**
 * Render the tile at destination (x, y).
 *
 * @param renderer the SDL renderer to use
 * @param font     the font texture
 * @param tile     the tile to render
 * @param x        the x-coordinate of the destination
 * @param y        the y-coordinate of the destination
 *
 * @return whether rendering was completely successful.
 */
bool
rl_draw_tile(SDL_Renderer* renderer,
             SDL_Texture* font,
             struct rl_gfx_tile const* tile,
             float x,
             float y);

#endif // GINC_ROGUELIKE_RENDER_H
