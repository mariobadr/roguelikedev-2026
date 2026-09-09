#include "render.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include "cell.h"
#include "font.h"
#include "graphics.h"
#include "text.h"

/**
 * Calculate the source rectangle of a glyph in the font.
 *
 * @param font  the font to look up the glyph in
 * @param glyph the glyph from the font
 *
 * @return the location of the glyph in the font texture.
 */
static SDL_FRect
calculate_source(struct rl_font const* font, Uint8 glyph)
{
  SDL_FRect src = { 0 };
  src.x = (glyph % font->columns) * (float)font->glyph_width;
  src.y = (glyph / font->columns) * (float)font->glyph_height;
  src.w = (float)font->glyph_width;
  src.h = (float)font->glyph_height;

  return src;
}

/**
 * Calculate the destination rectangle of a cell on screen.
 *
 * @param col the column of the cell
 * @param row the row of the cell
 *
 * @return the area the cell occupies in logical pixels.
 */
static SDL_FRect
calculate_destination(int col, int row)
{
  SDL_Point cell = { 0 };
  cell.x = col;
  cell.y = row;

  SDL_FPoint const pixels = rl_cell_point_to_pixels(&cell);

  SDL_FRect dst = { 0 };
  dst.x = pixels.x;
  dst.y = pixels.y;
  dst.w = (float)rl_cell_width();
  dst.h = (float)rl_cell_height();

  return dst;
}

void
rl_fill_tile(SDL_Renderer* renderer, SDL_FColor colour, int col, int row)
{
  SDL_FRect const dst = calculate_destination(col, row);

  SDL_SetRenderDrawColorFloat(renderer, colour.r, colour.g, colour.b, colour.a);
  SDL_RenderFillRect(renderer, &dst);
}

void
rl_draw_tile(SDL_Renderer* renderer,
             struct rl_font const* font,
             struct rl_gfx_tile const* tile,
             int col,
             int row)
{
  SDL_FRect const dst = calculate_destination(col, row);

  if (tile->bg.a > 0.0f) {
    rl_fill_tile(renderer, tile->bg, col, row);
  }

  // tint the glyph (foreground)
  SDL_SetTextureColorModFloat(
    font->texture, tile->fg.r, tile->fg.g, tile->fg.b);
  SDL_SetTextureAlphaModFloat(font->texture, tile->fg.a);

  // draw the glyph
  SDL_FRect src = calculate_source(font, tile->glyph);
  SDL_RenderTexture(renderer, font->texture, &src, &dst);
}

void
rl_draw_string(SDL_Renderer* renderer,
               struct rl_font const* font,
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
             struct rl_font const* font,
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
