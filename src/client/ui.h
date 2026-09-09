/**
 * @file ui.h
 *
 * Roughly:
 *
 * ┌──────────────────────────────┬───────────────┐
 * │          top panel           │               │
 * ├──────────────────────────────┤               │
 * │                              │  right panel  │
 * │            main              │               │
 * │                              │               │
 * ├──────────────────────────────┤               │
 * │         bottom panel         │               │
 * └──────────────────────────────┴───────────────┘
 */
#ifndef GINC_ROGUELIKE_UI_H
#define GINC_ROGUELIKE_UI_H

#include <SDL3/SDL_rect.h>

/** The width of the entire UI, in logical pixels. */
#define RL_UI_WIDTH 480
/** The height of the entire UI, in logical pixels. */
#define RL_UI_HEIGHT 360

struct rl_ui_layout
{
  SDL_FRect main_panel;
  SDL_FRect top_panel;
  SDL_FRect bottom_panel;
  SDL_FRect right_panel;
  SDL_FRect left_panel;
};

void
rl_init_ui_layout(struct rl_ui_layout* layout);

SDL_FPoint
rl_panel_to_pixels(SDL_FRect const* panel, SDL_Point cell);

bool
rl_pixels_to_panel(SDL_FRect const* panel, SDL_FPoint at, SDL_FPoint* local);

SDL_Rect
rl_panel_clip_rect(SDL_FRect const* panel);

#endif // GINC_ROGUELIKE_UI_H
