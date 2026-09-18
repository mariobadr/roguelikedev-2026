/**
 * @file gameplay.h
 */
#ifndef GINC_ROGUELIKE_GAMEPLAY_SCREEN_H
#define GINC_ROGUELIKE_GAMEPLAY_SCREEN_H

#include <SDL3/SDL_stdinc.h>

// forward declarations
struct gfx_tileset;
struct rl_game;
struct rl_screen;

bool
rl_alloc_gameplay_screen(struct rl_screen* screen,
                         struct gfx_tileset const* font,
                         struct rl_game* game);

#endif // GINC_ROGUELIKE_GAMEPLAY_SCREEN_H
