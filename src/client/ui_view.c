#include "ui_view.h"

#include <SDL3/SDL_render.h>

#include "game/actor.h"

#include "client/font.h"
#include "client/game_log.h"
#include "client/palette.h"
#include "client/render.h"
#include "client/text.h"
#include "client/ui.h"

void
rl_draw_status(SDL_Renderer* renderer,
               struct rl_font const* font,
               SDL_FRect const* panel,
               struct rl_actor const* rogue)
{
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  SDL_Rect const clip = rl_panel_clip_rect(panel);
  SDL_SetRenderClipRect(renderer, &clip);

  char text[16];
  SDL_snprintf(text, sizeof(text), "HP: %d / %d", rogue->hp, rogue->max_hp);

  SDL_FPoint const at = { panel->x, panel->y };
  rl_draw_string(renderer, font, text, RL_COLOUR_GRAY[5], RL_COLOUR_BLACK, at);

  SDL_SetRenderClipRect(renderer, NULL);
}
