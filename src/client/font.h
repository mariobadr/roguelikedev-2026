/**
 * @file font.h
 */
#ifndef GINC_ROGUELIKE_FONT_H
#define GINC_ROGUELIKE_FONT_H

#include <SDL3/SDL_stdinc.h>

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;
typedef struct SDL_Texture SDL_Texture;

struct rl_font
{
  /** Texture holding the glyphs. */
  SDL_Texture* texture;
  /** The width of one glyph, in pixels. */
  int glyph_width;
  /** The height of one glyph, in pixels. */
  int glyph_height;
  /** The number of glyphs per row. */
  int columns;
};

/**
 * Loads the bitmap font used to render the game.
 *
 * @return whether the font was loaded successfully.
 */
bool
rl_load_font(struct rl_font* font, SDL_Renderer* renderer);

/**
 * Unload a previously loaded font.
 */
void
rl_unload_font(struct rl_font* font);

/**
 * @return the width a string with glyph_count needs, in pixels.
 */
float
rl_font_width(struct rl_font const* font, size_t glyph_count);

#endif // GINC_ROGUELIKE_FONT_H
