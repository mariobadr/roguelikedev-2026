/**
 * @file render.h
 */
#ifndef GINC_ROGUELIKE_RENDER_H
#define GINC_ROGUELIKE_RENDER_H

#include <SDL3/SDL_stdinc.h>

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;

#define GLYPH_WIDTH (6.0f)
#define GLYPH_HEIGHT (8.0f)
#define FONT_ROWS (16)
#define FONT_COLS (16)

/**
 * Render the glyph from font at destination (x, y).
 *
 * @param renderer the SDL renderer to use
 * @param font     the font texture
 * @param glyph    the glyph to render
 * @param x        the x-coordinate of the destination
 * @param y        the y-coordinate of the destination
 *
 * @return whether rendering was successful.
 */
bool
rl_render_glyph(SDL_Renderer* renderer,
                SDL_Texture* font,
                char glyph,
                float x,
                float y);

#endif // GINC_ROGUELIKE_RENDER_H
