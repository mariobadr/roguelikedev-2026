#include "render.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

/**
 * Calculate the source rectangle of a glyph in the font.
 *
 * @param glyph the glyph from the font
 *
 * @return the location of the glyph in the font texture.
 */
SDL_FRect
calculate_source(Uint8 glyph)
{
  SDL_FRect src = { 0 };
  src.x = (glyph % FONT_COLS) * GLYPH_WIDTH;
  src.y = (glyph / FONT_COLS) * GLYPH_HEIGHT;
  src.w = GLYPH_WIDTH;
  src.h = GLYPH_HEIGHT;

  return src;
}

bool
rl_draw_tile(SDL_Renderer* renderer,
             SDL_Texture* font,
             struct rl_gfx_tile const* tile,
             float x,
             float y)
{
  bool ok = true;

  SDL_FRect dst = { 0 };
  dst.x = x;
  dst.y = y;
  dst.w = GLYPH_WIDTH;
  dst.h = GLYPH_HEIGHT;

  if (tile->bg.a > 0.0f) {
    // draw the background
    ok &= SDL_SetRenderDrawColorFloat(
      renderer, tile->bg.r, tile->bg.g, tile->bg.b, tile->bg.a);
    ok &= SDL_RenderFillRect(renderer, &dst);
  }

  // tint the glyph (foreground)
  ok &= SDL_SetTextureColorModFloat(font, tile->fg.r, tile->fg.g, tile->fg.b);
  ok &= SDL_SetTextureAlphaModFloat(font, tile->fg.a);

  // draw the glyph
  SDL_FRect src = calculate_source(tile->glyph);
  ok &= SDL_RenderTexture(renderer, font, &src, &dst);

  return ok;
}
