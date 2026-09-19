#include "console.h"

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_render.h>

#include "tileset.h"

static void
draw_grid(SDL_Renderer* renderer,
          struct gfx_tileset const* tileset,
          grid(gfx_console) const* console,
          SDL_Rect const* region,
          SDL_FPoint origin)
{
  for (int y = 0; y < region->h; ++y) {
    for (int x = 0; x < region->w; ++x) {
      SDL_FPoint at = { 0 };
      at.x = origin.x + x * tileset->tile_width;
      at.y = origin.y + y * tileset->tile_height;
      SDL_FRect dst = gfx_tileset_dst(tileset, at, 1);

      struct gfx_console_cell const* cell =
        grid_at(console, region->x + x, region->y + y);
      gfx_draw_console_cell(renderer, tileset, cell, &dst);
    }
  }
}

void
gfx_draw_console_cell(SDL_Renderer* renderer,
                      struct gfx_tileset const* tileset,
                      struct gfx_console_cell const* cell,
                      SDL_FRect const* dst)
{
  if (cell->bg.a > 0.0f) {
    SDL_SetRenderDrawColorFloat(
      renderer, cell->bg.r, cell->bg.g, cell->bg.b, cell->bg.a);
    SDL_RenderFillRect(renderer, dst);
  }

  if (cell->fg.a > 0.0f) {
    // tint the glyph (foreground)
    SDL_SetTextureColorModFloat(
      tileset->texture, cell->fg.r, cell->fg.g, cell->fg.b);
    SDL_SetTextureAlphaModFloat(tileset->texture, cell->fg.a);

    // draw the glyph
    SDL_FRect src = gfx_tileset_src(tileset, cell->index);
    SDL_RenderTexture(renderer, tileset->texture, &src, dst);
  }
}

void
gfx_clear_console_grid(grid(gfx_console) * console)
{
  SDL_memset(console->data, 0, grid_count(console) * sizeof(*console->data));
}

void
gfx_draw_console_grid(SDL_Renderer* renderer,
                      struct gfx_tileset const* tileset,
                      grid(gfx_console) const* console,
                      SDL_Rect const* region,
                      SDL_FPoint at)
{
  SDL_Rect all = { 0 };
  all.w = grid_width(console);
  all.h = grid_height(console);

  if (region == NULL) {
    region = &all;
  }

  draw_grid(renderer, tileset, console, region, at);
}

void
gfx_print_console(SDL_Renderer* renderer,
                  struct gfx_tileset const* font,
                  struct str_view text,
                  SDL_FColor fg,
                  SDL_FColor bg,
                  SDL_FPoint at)
{
  struct gfx_console_cell cell = { 0 };
  cell.fg = fg;
  cell.bg = bg;

  for (int i = 0; i < text.length; ++i) {
    cell.index = (Uint8)text.data[i];

    SDL_FRect dst = gfx_tileset_dst(font, at, 1);
    gfx_draw_console_cell(renderer, font, &cell, &dst);

    at.x += font->tile_width;
  }
}
