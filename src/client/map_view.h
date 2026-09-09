/**
 * @file map_view.h
 */
#ifndef GINC_ROGUELIKE_MAP_VIEW_H
#define GINC_ROGUELIKE_MAP_VIEW_H

#include <SDL3/SDL_rect.h>

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct rl_font;
struct rl_fov;
struct rl_world;

void
rl_draw_map(SDL_Renderer* renderer,
            struct rl_font const* font,
            SDL_FRect const* panel,
            struct rl_world const* world,
            struct rl_fov const* fov);

#endif // GINC_ROGUELIKE_MAP_VIEW_H
