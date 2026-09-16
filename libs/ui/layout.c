#include "layout.h"

void
ui_layout_column(struct ui_position p,
                 SDL_FRect const* bounds,
                 struct ui_strip strip,
                 SDL_FRect* rects)
{
  float const total_height =
    strip.count * strip.item_height + (strip.count - 1) * strip.gap;

  SDL_FRect const column =
    ui_resolve(p, bounds, strip.item_width, total_height);

  for (int i = 0; i < strip.count; i++) {
    rects[i] = (SDL_FRect){
      .x = column.x,
      .y = column.y + i * (strip.item_height + strip.gap),
      .w = strip.item_width,
      .h = strip.item_height,
    };
  }
}

void
ui_layout_row(struct ui_position p,
              SDL_FRect const* bounds,
              struct ui_strip strip,
              SDL_FRect* rects)
{
  float const total_width =
    strip.count * strip.item_width + (strip.count - 1) * strip.gap;

  SDL_FRect const row = ui_resolve(p, bounds, total_width, strip.item_height);

  for (int i = 0; i < strip.count; i++) {
    rects[i] = (SDL_FRect){
      .x = row.x + i * (strip.item_width + strip.gap),
      .y = row.y,
      .w = strip.item_width,
      .h = strip.item_height,
    };
  }
}
