#include "ui_view.h"

#include <SDL3/SDL_render.h>

#include "game/actor.h"

#include "client/font.h"
#include "client/game_log.h"
#include "client/palette.h"
#include "client/render.h"
#include "client/text.h"
#include "client/ui.h"

static float
line_advance(struct rl_font const* font)
{
  return (float)font->glyph_height;
}

void
rl_draw_log(SDL_Renderer* renderer,
            struct rl_font const* font,
            SDL_FRect const* panel,
            struct rl_game_log const* log)
{
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  SDL_Rect const clip = rl_panel_clip_rect(panel);
  SDL_SetRenderClipRect(renderer, &clip);

  int const len = (int)alist_len(&log->messages);
  int const capacity = SDL_max(0, (int)(panel->h / line_advance(font)));

  // no scrolling controls yet, so show only the latest messages
  int const start = SDL_max(0, len - capacity);

  int row = 0;
  for (int i = start; i < len; i++) {
    struct rl_text const* message = alist_at(&log->messages, i);
    SDL_FPoint const at = {
      panel->x, panel->y + (float)row * line_advance(font)
    };
    rl_draw_text(renderer, font, message, at);
    row += 1;
  }

  SDL_SetRenderClipRect(renderer, NULL);
}

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

void
rl_draw_controls(SDL_Renderer* renderer,
                 struct rl_font const* font,
                 SDL_FRect const* panel)
{
  SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

  SDL_Rect const clip = rl_panel_clip_rect(panel);
  SDL_SetRenderClipRect(renderer, &clip);

  char const* text = "\x18 W | \x1B A | \x19 S | \x1A D";

  SDL_FPoint const at = { panel->x, panel->y };
  rl_draw_string(renderer, font, text, RL_COLOUR_GRAY[5], RL_COLOUR_BLACK, at);

  SDL_SetRenderClipRect(renderer, NULL);
}
