/**
 * @file view.h
 */
#ifndef GINC_ROGUELIKE_VIEW_H
#define GINC_ROGUELIKE_VIEW_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct rl_actor;
struct rl_font;
struct rl_fov;
struct rl_game_log;
struct rl_world;

void
rl_draw_map(SDL_Renderer* renderer,
            struct rl_font const* font,
            SDL_Rect const* panel,
            struct rl_world const* world,
            struct rl_fov const* fov);

void
rl_draw_log(SDL_Renderer* renderer,
            struct rl_font const* font,
            SDL_Rect const* panel,
            struct rl_game_log const* log);

void
rl_draw_status(SDL_Renderer* renderer,
               struct rl_font const* font,
               SDL_Rect const* panel,
               struct rl_actor const* rogue);

void
rl_draw_controls(SDL_Renderer* renderer,
                 struct rl_font const* font,
                 SDL_Rect const* panel);

#endif // GINC_ROGUELIKE_VIEW_H
