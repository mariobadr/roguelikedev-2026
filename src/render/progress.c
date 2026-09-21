#include "progress.h"

#include <SDL3/SDL_render.h>

void
rl_draw_progress(SDL_Renderer* renderer,
                 struct ui_progress const* bar,
                 SDL_FColor fg,
                 SDL_FColor bg,
                 SDL_FPoint at)
{
  SDL_FRect const track = { at.x, at.y, bar->size.x, bar->size.y };
  SDL_SetRenderDrawColorFloat(renderer, bg.r, bg.g, bg.b, bg.a);
  SDL_RenderFillRect(renderer, &track);

  SDL_FRect const fill = { at.x, at.y, bar->fill, bar->size.y };
  SDL_SetRenderDrawColorFloat(renderer, fg.r, fg.g, fg.b, fg.a);
  SDL_RenderFillRect(renderer, &fill);
}
