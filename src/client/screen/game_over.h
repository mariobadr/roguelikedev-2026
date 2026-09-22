/**
 * @file game_over.h
 */
#ifndef GINC_ROGUELIKE_GAME_OVER_SCREEN_H
#define GINC_ROGUELIKE_GAME_OVER_SCREEN_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "save/format.h"

// forward declarations
struct gfx_tileset;
struct rl_screen;

/**
 * Allocate the screen shown when a run has ended with outcome.
 *
 * @param outcome must be RL_RUN_DEAD or RL_RUN_VICTORY.
 *
 * @return whether allocation succeeded.
 */
bool
rl_alloc_game_over_screen(struct rl_screen* screen,
                          SDL_FRect const* bounds,
                          struct gfx_tileset const* font,
                          enum rl_run_outcome outcome);

#endif // GINC_ROGUELIKE_GAME_OVER_SCREEN_H
