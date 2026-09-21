/**
 * @file progress.h
 */
#ifndef GINC_UI_PROGRESS_H
#define GINC_UI_PROGRESS_H

#include <SDL3/SDL_rect.h>

/**
 * The size of a horizontal progress bar and how much of it is filled.
 */
struct ui_progress
{
  /** The size of the whole bar, in pixels. */
  SDL_FPoint size;
  /** The width of the filled part, from the left, in pixels. */
  float fill;
};

/**
 * @return a bar of size, filled in proportion to value out of max.
 */
struct ui_progress
ui_progress_layout(SDL_FPoint size, int value, int max);

#endif // GINC_UI_PROGRESS_H
