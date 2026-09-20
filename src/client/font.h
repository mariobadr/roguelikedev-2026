/**
 * @file font.h
 */
#ifndef GINC_ROGUELIKE_FONT_H
#define GINC_ROGUELIKE_FONT_H

#include <SDL3/SDL_stdinc.h>

#include "graphics/tileset.h"

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

/**
 * Load the bitmap font used to render the game.
 *
 * @return whether the font was loaded successfully.
 */
bool
rl_load_font(struct gfx_tileset* font, SDL_Renderer* renderer);

/**
 * Unload a previously loaded font.
 */
void
rl_unload_font(struct gfx_tileset* font);

/**
 * @return the width a string with glyph_count needs, in pixels.
 */
float
rl_font_width(struct gfx_tileset const* font, size_t glyph_count);

#endif // GINC_ROGUELIKE_FONT_H
