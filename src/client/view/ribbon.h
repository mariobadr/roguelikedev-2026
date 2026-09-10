/**
 * @file ribbon.h
 */
#ifndef GINC_ROGUELIKE_RIBBON_VIEW_H
#define GINC_ROGUELIKE_RIBBON_VIEW_H

#include <SDL3/SDL_rect.h>

#include "client/text.h"

typedef struct SDL_Renderer SDL_Renderer;
struct rl_font;

enum rl_ribbon_slot
{
  RL_RIBBON_LEFT,
  RL_RIBBON_CENTRE,
  RL_RIBBON_RIGHT,
  RL_RIBBON_SLOT_COUNT,
};

struct rl_ribbon
{
  struct rl_text text[RL_RIBBON_SLOT_COUNT];
  SDL_FRect slots[RL_RIBBON_SLOT_COUNT];
};

void
rl_init_ribbon(struct rl_ribbon* ribbon,
               SDL_FRect const* viewport);

void
rl_resize_ribbon(struct rl_ribbon* ribbon,
                 SDL_FRect const* viewport);

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
               struct rl_font const* font);

#endif // GINC_ROGUELIKE_RIBBON_VIEW_H
