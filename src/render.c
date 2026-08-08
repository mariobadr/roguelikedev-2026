#include "render.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

bool
rl_render_glyph(SDL_Renderer* renderer,
                SDL_Texture* font,
                char glyph,
                float x,
                float y)
{
  // get a 0 to 255 index into the font
  unsigned char const index = glyph;

  SDL_FRect src = { 0 };
  src.x = (index % FONT_COLS) * GLYPH_WIDTH;
  src.y = (index / FONT_COLS) * GLYPH_HEIGHT;
  src.w = GLYPH_WIDTH;
  src.h = GLYPH_HEIGHT;

  SDL_FRect dst = { 0 };
  dst.x = x;
  dst.y = y;
  dst.w = GLYPH_WIDTH;
  dst.h = GLYPH_HEIGHT;

  return SDL_RenderTexture(renderer, font, &src, &dst);
}
