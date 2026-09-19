#include "ribbon.h"

#include <SDL3/SDL_render.h>

#include "ui/anchor.h"

#include "render/palette.h"

#include "client/font.h"
#include "client/text.h"

// Space between adjacent occupied slots, in glyph widths.
#define RIBBON_GAP_GLYPHS 2.0f

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

static float
text_width(struct rl_ribbon const* ribbon, enum rl_ribbon_slot slot)
{
  return (float)ribbon->content.text[slot].length * ribbon->glyph_width;
}

// Sizes the slots by priority (right, left, centre) so that the slots never
// overlap or extend beyond the viewport.
static void
layout_slots(struct rl_ribbon* ribbon, SDL_FRect slots[RL_RIBBON_SLOT_COUNT])
{
  SDL_FRect const* viewport = &ribbon->viewport;
  float const width = SDL_max(viewport->w, 0.0f);
  float const height = SDL_max(viewport->h, 0.0f);
  float const gap = RIBBON_GAP_GLYPHS * ribbon->glyph_width;
  float const left_edge = viewport->x;
  float const right_edge = viewport->x + width;

  float const right_w = SDL_min(text_width(ribbon, RL_RIBBON_RIGHT), width);
  float const right_gap = right_w > 0.0f ? gap : 0.0f;

  float const left_room = SDL_max(width - right_w - right_gap, 0.0f);
  float const left_w = SDL_min(text_width(ribbon, RL_RIBBON_LEFT), left_room);
  float const left_gap = left_w > 0.0f ? gap : 0.0f;

  // the centre slot spans everything between the other two
  float const centre_x = left_edge + left_w + left_gap;
  float const centre_room =
    SDL_max(right_edge - right_w - right_gap - centre_x, 0.0f);
  float const centre_w =
    ribbon->content.text[RL_RIBBON_CENTRE].length > 0 ? centre_room : 0.0f;

  slots[RL_RIBBON_LEFT] = (SDL_FRect){ left_edge, viewport->y, left_w, height };
  slots[RL_RIBBON_CENTRE] =
    (SDL_FRect){ centre_x, viewport->y, centre_w, height };
  slots[RL_RIBBON_RIGHT] =
    (SDL_FRect){ right_edge - right_w, viewport->y, right_w, height };

  for (int slot = 0; slot < RL_RIBBON_SLOT_COUNT; ++slot) {
    ribbon->clips[slot] = frect_to_rect(&slots[slot]);
  }
}

// Text wider than its slot starts at the slot's left edge and is clipped.
static void
place_text(struct rl_ribbon* ribbon,
           enum rl_ribbon_slot slot,
           SDL_FRect const* bounds)
{
  struct ui_position const pos = { .anchor = SLOT_ANCHORS[slot] };
  float const width = SDL_min(text_width(ribbon, slot), bounds->w);

  SDL_FRect const dst = ui_resolve(pos, bounds, width, ribbon->line_height);
  ribbon->origins[slot] = (SDL_FPoint){ dst.x, dst.y };
}

void
rl_init_ribbon(struct rl_ribbon* ribbon,
               SDL_FRect const* viewport,
               struct gfx_tileset const* font)
{
  ribbon->viewport = *viewport;
  ribbon->glyph_width = (float)font->tile_width;
  ribbon->line_height = (float)font->tile_height;

  struct rl_ribbon_content const empty = { 0 };
  rl_set_ribbon_content(ribbon, &empty);
}

void
rl_set_ribbon_content(struct rl_ribbon* ribbon,
                      struct rl_ribbon_content const* content)
{
  ribbon->content = *content;

  SDL_FRect slots[RL_RIBBON_SLOT_COUNT];
  layout_slots(ribbon, slots);
  for (int slot = 0; slot < RL_RIBBON_SLOT_COUNT; ++slot) {
    place_text(ribbon, slot, &slots[slot]);
  }
}

void
rl_draw_ribbon(struct rl_ribbon const* ribbon,
               SDL_Renderer* renderer,
               struct gfx_tileset const* font)
{
  for (int slot = 0; slot < RL_RIBBON_SLOT_COUNT; ++slot) {
    struct rl_text const* text = &ribbon->content.text[slot];
    SDL_Rect const* clip = &ribbon->clips[slot];
    if (text->length == 0 || clip->w <= 0 || clip->h <= 0) {
      continue;
    }

    SDL_SetRenderClipRect(renderer, clip);
    rl_draw_text(renderer,
                 font,
                 text,
                 RL_COLOUR_GRAY[5],
                 RL_COLOUR_BLACK,
                 ribbon->origins[slot]);
  }

  SDL_SetRenderClipRect(renderer, NULL);
}
