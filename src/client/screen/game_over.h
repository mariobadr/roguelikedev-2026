/**
 * @file game_over.h
 */
#ifndef GINC_ROGUELIKE_GAME_OVER_SCREEN_H
#define GINC_ROGUELIKE_GAME_OVER_SCREEN_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// forward declarations
struct gfx_tileset;
struct rl_screen;

/**
 * Allocate the screen shown when the rogue has died.
 *
 * @return whether allocation succeeded.
 */
bool
rl_alloc_game_over_screen(struct rl_screen* screen,
                          SDL_FRect const* bounds,
                          struct gfx_tileset const* font);

#endif // GINC_ROGUELIKE_GAME_OVER_SCREEN_H
