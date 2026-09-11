#include "ribbon.h"

#include <SDL3/SDL_render.h>

#include "ui/layout.h"

#include "client/font.h"
#include "client/palette.h"
#include "client/render.h"
#include "client/ui.h"

void
rl_init_ribbon(struct rl_ribbon* ribbon, SDL_FRect const* viewport)
{
  rl_resize_ribbon(ribbon, viewport);
}

void
rl_resize_ribbon(struct rl_ribbon* ribbon, SDL_FRect const* viewport)
{
  struct ui_strip strip = { 0 };
  strip.count = RL_RIBBON_SLOT_COUNT;
  strip.item_width = viewport->w / RL_RIBBON_SLOT_COUNT;
  strip.item_height = viewport->h;
  strip.gap = 0.0f;

  struct ui_position const pos = { .anchor = UI_ANCHOR_TOP_LEFT };
  ui_layout_row(pos, viewport, strip, ribbon->slots);
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
}

void
rl_draw_ribbon(struct rl_ribbon const* ribbon,
               SDL_Renderer* renderer,
               struct rl_font const* font)
{
  Uint8 const anchors[RL_RIBBON_SLOT_COUNT] = {
    UI_ANCHOR_MID_LEFT,
    UI_ANCHOR_CENTRE,
    UI_ANCHOR_MID_RIGHT,
  };

  for (int slot = 0; slot < RL_RIBBON_SLOT_COUNT; ++slot) {
    struct rl_text const* text = &ribbon->text[slot];
    if (text->length == 0) {
      continue;
    }

    SDL_FRect const* bounds = &ribbon->slots[slot];
    struct ui_position const pos = { .anchor = anchors[slot] };
    SDL_FRect const dst = ui_resolve(
      pos, bounds, rl_font_width(font, text->length), (float)font->glyph_height);
    SDL_FPoint const at = { dst.x, dst.y };
    SDL_Rect const clip = rl_panel_clip_rect(bounds);

    SDL_SetRenderClipRect(renderer, &clip);
    rl_draw_text(renderer, font, text, RL_COLOUR_GRAY[5], RL_COLOUR_BLACK, at);
  }

  SDL_SetRenderClipRect(renderer, NULL);
}
