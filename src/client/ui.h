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

/** The width of the entire UI. */
#define RL_UI_WIDTH 80
/** The height of the entire UI. */
#define RL_UI_HEIGHT 45

struct rl_ui_layout
{
  SDL_Rect main_panel;
  SDL_Rect top_panel;
  SDL_Rect bottom_panel;
  SDL_Rect right_panel;
  SDL_Rect left_panel;
};

void
rl_init_ui_layout(struct rl_ui_layout* layout);

SDL_Point
rl_panel_to_cell(SDL_Rect const* panel, SDL_Point local);

SDL_FPoint
rl_panel_to_pixels(SDL_Rect const* panel, SDL_Point local);

bool
rl_cell_to_panel(SDL_Rect const* panel, SDL_Point at, SDL_Point* local);

#endif // GINC_ROGUELIKE_UI_H
