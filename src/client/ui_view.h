/**
 * @file ui_view.h
 */
#ifndef GINC_ROGUELIKE_UI_VIEW_H
#define GINC_ROGUELIKE_UI_VIEW_H

#include <SDL3/SDL_rect.h>

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct rl_actor;
struct rl_font;
struct rl_game_log;

void
rl_draw_status(SDL_Renderer* renderer,
               struct rl_font const* font,
               SDL_FRect const* panel,
               struct rl_actor const* rogue);

#endif // GINC_ROGUELIKE_UI_VIEW_H
