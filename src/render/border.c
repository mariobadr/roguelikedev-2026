#include "border.h"

#include "graphics/console.h"
#include "graphics/tileset.h"

static void
draw_border_cell(SDL_Renderer* renderer,
                 struct gfx_tileset const* font,
                 struct gfx_console_cell* output,
                 struct cp437_border_cell const* border,
                 int x,
                 int y,
                 SDL_FPoint origin)
{
  if (border->connections == 0) {
    return;
  }

  SDL_FPoint const at = {
    origin.x + (float)x * (float)font->tile_width,
    origin.y + (float)y * (float)font->tile_height,
  };
  SDL_FRect const dst = gfx_tileset_dst(font, at, 1);

  output->index = cp437_border_glyph(border);
  gfx_draw_console_cell(renderer, font, output, &dst);
}

void
rl_draw_cp437_box(SDL_Renderer* renderer,
                  struct gfx_tileset const* font,
                  SDL_Rect const* box,
                  struct cp437_border_style const* style,
                  SDL_FColor fg,
                  SDL_FColor bg,
                  SDL_FPoint origin)
{
  struct gfx_console_cell output = { 0 };
  output.fg = fg;
  output.bg = bg;

  int const right = box->x + box->w - 1;
  int const bottom = box->y + box->h - 1;

  for (int x = box->x; x <= right; ++x) {
    struct cp437_border_cell const top_cell =
      cp437_box_cell(box, style, x, box->y);
    struct cp437_border_cell const bottom_cell =
      cp437_box_cell(box, style, x, bottom);

    draw_border_cell(renderer, font, &output, &top_cell, x, box->y, origin);
    draw_border_cell(renderer, font, &output, &bottom_cell, x, bottom, origin);
  }

  for (int y = box->y + 1; y < bottom; ++y) {
    struct cp437_border_cell const left_cell =
      cp437_box_cell(box, style, box->x, y);
    struct cp437_border_cell const right_cell =
      cp437_box_cell(box, style, right, y);

    draw_border_cell(renderer, font, &output, &left_cell, box->x, y, origin);
    draw_border_cell(renderer, font, &output, &right_cell, right, y, origin);
  }
}

void
rl_draw_cp437_borders(SDL_Renderer* renderer,
                      struct gfx_tileset const* font,
                      grid(cp437_border) const* borders,
                      SDL_FColor fg,
                      SDL_FColor bg,
                      SDL_FPoint origin)
{
  struct gfx_console_cell output = {
    .fg = fg,
    .bg = bg,
  };

  for (int y = 0; y < grid_height(borders); ++y) {
    for (int x = 0; x < grid_width(borders); ++x) {
      draw_border_cell(
        renderer, font, &output, grid_at(borders, x, y), x, y, origin);
    }
  }
}
