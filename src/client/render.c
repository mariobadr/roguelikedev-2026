#include "render.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>
#include <SDL3/SDL_stdinc.h>

#include "graphics/console.h"
#include "graphics/tileset.h"

#include "text.h"

static SDL_FColor
next_span_colour(struct rl_text const* text,
                 size_t* span,
                 size_t i,
                 SDL_FColor def)
{
  while (*span < text->span_count && text->spans[*span].end <= i) {
    (*span)++;
  }

  if (*span < text->span_count && i >= text->spans[*span].start) {
    return text->spans[*span].colour;
  }

  return def;
}

void
rl_draw_string(SDL_Renderer* renderer,
               struct gfx_tileset const* font,
               char const* text,
               SDL_FColor fg,
               SDL_FColor bg,
               SDL_FPoint at)
{
  struct gfx_console_cell cell = { 0 };
  cell.fg = fg;
  cell.bg = bg;

  size_t const length = SDL_strlen(text);
  for (size_t i = 0; i < length; i++) {
    cell.index = (Uint8)text[i];

    SDL_FRect dst = gfx_tileset_dst(font, at, 1);
    gfx_draw_cell(renderer, font, &cell, &dst);

    at.x += font->tile_width;
  }
}

void
rl_draw_text(SDL_Renderer* renderer,
             struct gfx_tileset const* font,
             struct rl_text const* text,
             SDL_FColor fg,
             SDL_FColor bg,
             SDL_FPoint at)
{
  struct gfx_console_cell cell = { 0 };
  cell.bg = bg;

  size_t span = 0;
  for (size_t i = 0; i < text->length; i++) {
    cell.fg = next_span_colour(text, &span, i, fg);
    cell.index = (Uint8)text->content[i];

    SDL_FRect dst = gfx_tileset_dst(font, at, 1);
    gfx_draw_cell(renderer, font, &cell, &dst);

    at.x += font->tile_width;
  }
}
