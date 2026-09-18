/**
 * @file ribbon.h
 */
#ifndef GINC_ROGUELIKE_RIBBON_VIEW_H
#define GINC_ROGUELIKE_RIBBON_VIEW_H

#include <SDL3/SDL_rect.h>

#include "client/text.h"

typedef struct SDL_Renderer SDL_Renderer;
struct gfx_tileset;

enum rl_ribbon_slot
{
  RL_RIBBON_LEFT,
  RL_RIBBON_CENTRE,
  RL_RIBBON_RIGHT,
  RL_RIBBON_SLOT_COUNT,
};

struct rl_ribbon
{
  // Static geometry, fixed by init.
  SDL_FRect slots[RL_RIBBON_SLOT_COUNT];
  SDL_Rect clips[RL_RIBBON_SLOT_COUNT];
  float glyph_width;
  float line_height;

  // Presentation data, updated whenever a slot's text is set.
  struct rl_text text[RL_RIBBON_SLOT_COUNT];
  SDL_FPoint origins[RL_RIBBON_SLOT_COUNT];
};

/**
 * Lays out the ribbon slots within viewport. font is only read for its glyph
 * metrics; draw with the same font.
 */
void
rl_init_ribbon(struct rl_ribbon* ribbon,
               SDL_FRect const* viewport,
               struct gfx_tileset const* font);

void
rl_set_current_view(struct rl_ribbon* ribbon, char const* str);

void
rl_set_current_mode(struct rl_ribbon* ribbon, char const* str);

void
rl_set_ribbon_text(struct rl_ribbon* ribbon,
                   enum rl_ribbon_slot slot,
                   struct rl_text const* text);

void
rl_draw_ribbon(struct rl_ribbon const* ribbon,
               SDL_Renderer* renderer,
               struct gfx_tileset const* font);

#endif // GINC_ROGUELIKE_RIBBON_VIEW_H
