#include "ui.h"

SDL_Rect
rl_panel_clip_rect(SDL_FRect const* panel)
{
  return (
    SDL_Rect){ (int)panel->x, (int)panel->y, (int)panel->w, (int)panel->h };
}
