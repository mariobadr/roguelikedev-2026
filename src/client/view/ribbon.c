#include "ribbon.h"

#include <SDL3/SDL_render.h>

#include "ui/layout.h"

#include "client/font.h"
#include "client/palette.h"
#include "client/render.h"

static Uint8 const SLOT_ANCHORS[RL_RIBBON_SLOT_COUNT] = {
  UI_ANCHOR_MID_LEFT,
  UI_ANCHOR_CENTRE,
  UI_ANCHOR_MID_RIGHT,
};

static SDL_Rect
frect_to_rect(SDL_FRect const* bounds)
{
  SDL_Rect rect = { 0 };
  rect.x = (int)bounds->x;
  rect.y = (int)bounds->y;
  rect.w = (int)bounds->w;
  rect.h = (int)bounds->h;

  return rect;
}

static void
layout_ribbon(struct rl_ribbon* ribbon, SDL_FRect const* viewport)
{
  struct ui_strip strip = { 0 };
  strip.count = RL_RIBBON_SLOT_COUNT;
  strip.item_width = viewport->w / RL_RIBBON_SLOT_COUNT;
  strip.item_height = viewport->h;
  strip.gap = 0.0f;

  struct ui_position const pos = { .anchor = UI_ANCHOR_TOP_LEFT };
  ui_layout_row(pos, viewport, strip, ribbon->slots);

  for (int slot = 0; slot < RL_RIBBON_SLOT_COUNT; ++slot) {
    ribbon->clips[slot] = frect_to_rect(&ribbon->slots[slot]);
  }
}

static void
place_text(struct rl_ribbon* ribbon, enum rl_ribbon_slot slot)
{
  struct ui_position const pos = { .anchor = SLOT_ANCHORS[slot] };
  float const width = (float)ribbon->text[slot].length * ribbon->glyph_width;

  SDL_FRect const dst =
    ui_resolve(pos, &ribbon->slots[slot], width, ribbon->line_height);
  ribbon->origins[slot] = (SDL_FPoint){ dst.x, dst.y };
}

void
rl_init_ribbon(struct rl_ribbon* ribbon,
               SDL_FRect const* viewport,
               struct gfx_tileset const* font)
{
  ribbon->glyph_width = (float)font->tile_width;
  ribbon->line_height = (float)font->tile_height;

  layout_ribbon(ribbon, viewport);
}

void
rl_set_current_view(struct rl_ribbon* ribbon, char const* str)
{
  struct rl_text txt = { 0 };
  rl_append_text(&txt, NULL, "View: ");
  rl_append_text(&txt, NULL, str);

  rl_set_ribbon_text(ribbon, RL_RIBBON_LEFT, &txt);
}

void
rl_set_current_mode(struct rl_ribbon* ribbon, char const* str)
{
  struct rl_text txt = { 0 };
  rl_append_text(&txt, NULL, str);

  rl_set_ribbon_text(ribbon, RL_RIBBON_CENTRE, &txt);
}

void
rl_set_ribbon_text(struct rl_ribbon* ribbon,
                   enum rl_ribbon_slot slot,
                   struct rl_text const* text)
{
  ribbon->text[slot] = *text;
  place_text(ribbon, slot);
}

void
rl_draw_ribbon(struct rl_ribbon const* ribbon,
               SDL_Renderer* renderer,
               struct gfx_tileset const* font)
{
  for (int slot = 0; slot < RL_RIBBON_SLOT_COUNT; ++slot) {
    struct rl_text const* text = &ribbon->text[slot];
    if (text->length == 0) {
      continue;
    }

    SDL_SetRenderClipRect(renderer, &ribbon->clips[slot]);
    rl_draw_text(renderer,
                 font,
                 text,
                 RL_COLOUR_GRAY[5],
                 RL_COLOUR_BLACK,
                 ribbon->origins[slot]);
  }

  SDL_SetRenderClipRect(renderer, NULL);
}
