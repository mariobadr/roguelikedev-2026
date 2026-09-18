/**
 * @file ribbon.h
 */
#ifndef GINC_ROGUELIKE_RIBBON_H
#define GINC_ROGUELIKE_RIBBON_H

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

/**
 * The text of every ribbon slot, indexed by rl_ribbon_slot.
 *
 * An empty rl_text leaves its slot blank.
 */
struct rl_ribbon_content
{
  struct rl_text text[RL_RIBBON_SLOT_COUNT];
};

struct rl_ribbon
{
  SDL_FRect viewport;
  float glyph_width;
  float line_height;

  // Layout state
  SDL_Rect clips[RL_RIBBON_SLOT_COUNT];
  SDL_FPoint origins[RL_RIBBON_SLOT_COUNT];

  // Presentation state
  struct rl_ribbon_content content;
};

void
rl_init_ribbon(struct rl_ribbon* ribbon,
               SDL_FRect const* viewport,
               struct gfx_tileset const* font);

void
rl_set_ribbon_content(struct rl_ribbon* ribbon,
                      struct rl_ribbon_content const* content);

void
rl_draw_ribbon(struct rl_ribbon const* ribbon,
               SDL_Renderer* renderer,
               struct gfx_tileset const* font);

#endif // GINC_ROGUELIKE_RIBBON_H
