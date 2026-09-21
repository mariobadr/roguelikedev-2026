#include "progress.h"

#include <SDL3/SDL_stdinc.h>

struct ui_progress
ui_progress_layout(SDL_FPoint size, int value, int max)
{
  struct ui_progress bar = { 0 };
  bar.size = size;
  bar.fill = 0.0f;

  if (max > 0) {
    float const fraction = (float)SDL_clamp(value, 0, max) / (float)max;
    bar.fill = SDL_floorf(size.x * fraction);
  }

  return bar;
}
