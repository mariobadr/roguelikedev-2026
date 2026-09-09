#include "render.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include "font.h"
#include "graphics.h"
#include "text.h"

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

static SDL_FRect
calculate_destination(struct rl_font const* font, SDL_FPoint at)
{
  SDL_FRect dst = { 0 };
  dst.x = at.x;
  dst.y = at.y;
  dst.w = (float)font->glyph_width;
  dst.h = (float)font->glyph_height;

  return dst;
}

void
rl_draw_tile(SDL_Renderer* renderer,
             struct rl_font const* font,
             struct rl_gfx_tile const* tile,
             SDL_FPoint at)
{
  SDL_FRect const dst = calculate_destination(font, at);

  if (tile->bg.a > 0.0f) {
    SDL_SetRenderDrawColorFloat(
      renderer, tile->bg.r, tile->bg.g, tile->bg.b, tile->bg.a);
    SDL_RenderFillRect(renderer, &dst);
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
               SDL_FPoint at)
{
  struct rl_gfx_tile tile = { 0 };
  tile.fg = fg;
  tile.bg = bg;

  size_t const length = SDL_strlen(text);
  for (size_t i = 0; i < length; i++) {
    tile.glyph = text[i];
    rl_draw_tile(renderer, font, &tile, at);
    at.x += (float)font->glyph_width;
  }
}

void
rl_draw_text(SDL_Renderer* renderer,
             struct rl_font const* font,
             struct rl_text const* text,
             SDL_FPoint at)
{
  for (int i = 0; i < text->run_count; i++) {
    struct rl_text_run const* run = &text->runs[i];
    struct rl_gfx_tile gfx = rl_get_text_gfx(run->style);

    for (int j = 0; run->text[j] != '\0'; j++) {
      gfx.glyph = run->text[j];
      rl_draw_tile(renderer, font, &gfx, at);
      at.x += (float)font->glyph_width;
    }
  }
}
