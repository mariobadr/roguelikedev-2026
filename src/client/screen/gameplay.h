/**
 * @file gameplay.h
 */
#ifndef GINC_ROGUELIKE_GAMEPLAY_SCREEN_H
#define GINC_ROGUELIKE_GAMEPLAY_SCREEN_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// forward declarations
struct gfx_tileset;
struct rl_run;
struct rl_screen;

/**
 * Allocate the main gameplay screen, which plays run.
 *
 * @return whether allocation succeeded.
 */
bool
rl_alloc_gameplay_screen(struct rl_screen* screen,
                         SDL_FRect const* bounds,
                         struct gfx_tileset const* font,
                         struct rl_run* run);

#endif // GINC_ROGUELIKE_GAMEPLAY_SCREEN_H
