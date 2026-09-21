/**
 * @file progress.h
 */
#ifndef GINC_ROGUELIKE_RENDER_PROGRESS_H
#define GINC_ROGUELIKE_RENDER_PROGRESS_H

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>

#include "ui/progress.h"

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

/**
 * Draw a progress bar from the screen position at, filling in fg over bg.
 */
void
rl_draw_progress(SDL_Renderer* renderer,
                 struct ui_progress const* bar,
                 SDL_FColor fg,
                 SDL_FColor bg,
                 SDL_FPoint at);

#endif // GINC_ROGUELIKE_RENDER_PROGRESS_H
