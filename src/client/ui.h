/**
 * @file ui.h
 */
#ifndef GINC_ROGUELIKE_UI_H
#define GINC_ROGUELIKE_UI_H

#include <SDL3/SDL_rect.h>

/** The width of the entire UI, in logical pixels. */
#define RL_UI_WIDTH 480
/** The height of the entire UI, in logical pixels. */
#define RL_UI_HEIGHT 360

SDL_Rect
rl_panel_clip_rect(SDL_FRect const* panel);

#endif // GINC_ROGUELIKE_UI_H
