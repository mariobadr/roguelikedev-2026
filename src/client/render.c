#include "render.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include "graphics.h"
#include "text.h"

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

void
rl_fill_tile(SDL_Renderer* renderer, SDL_FColor colour, int col, int row)
{
  SDL_FRect const dst = {
    col * GLYPH_WIDTH, row * GLYPH_HEIGHT, GLYPH_WIDTH, GLYPH_HEIGHT
  };

  SDL_SetRenderDrawColorFloat(renderer, colour.r, colour.g, colour.b, colour.a);
  SDL_RenderFillRect(renderer, &dst);
}

void
rl_draw_tile(SDL_Renderer* renderer,
             SDL_Texture* font,
             struct rl_gfx_tile const* tile,
             int col,
             int row)
{
  SDL_FRect dst = { 0 };
  dst.x = col * GLYPH_WIDTH;
  dst.y = row * GLYPH_HEIGHT;
  dst.w = GLYPH_WIDTH;
  dst.h = GLYPH_HEIGHT;

  if (tile->bg.a > 0.0f) {
    rl_fill_tile(renderer, tile->bg, col, row);
  }

  // tint the glyph (foreground)
  SDL_SetTextureColorModFloat(font, tile->fg.r, tile->fg.g, tile->fg.b);
  SDL_SetTextureAlphaModFloat(font, tile->fg.a);

  // draw the glyph
  SDL_FRect src = calculate_source(tile->glyph);
  SDL_RenderTexture(renderer, font, &src, &dst);
}

void
rl_draw_string(SDL_Renderer* renderer,
               SDL_Texture* font,
               char const* text,
               SDL_FColor fg,
               SDL_FColor bg,
               int col,
               int row)
{
  struct rl_gfx_tile tile = { 0 };
  tile.fg = fg;
  tile.bg = bg;

  size_t const length = SDL_strlen(text);
  for (size_t i = 0; i < length; i++) {
    tile.glyph = text[i];
    rl_draw_tile(renderer, font, &tile, col + (int)i, row);
  }
}

void
rl_draw_text(SDL_Renderer* renderer,
             SDL_Texture* font,
             struct rl_text const* text,
             int col,
             int row)
{
  int k = 0;

  for (int i = 0; i < text->run_count; i++) {
    struct rl_text_run const* run = &text->runs[i];
    struct rl_gfx_tile gfx = rl_get_text_gfx(run->style);

    for (int j = 0; run->text[j] != '\0'; j++) {
      gfx.glyph = run->text[j];
      rl_draw_tile(renderer, font, &gfx, col + k, row);

      k++;
    }
  }
}
