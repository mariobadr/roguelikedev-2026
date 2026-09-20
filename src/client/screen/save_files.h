/**
 * @file save_files.h
 */
#ifndef GINC_ROGUELIKE_SAVE_FILES_SCREEN_H
#define GINC_ROGUELIKE_SAVE_FILES_SCREEN_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// forward declarations
struct gfx_tileset;
struct rl_run;
struct rl_screen;

/**
 * Allocate a screen for browsing saved runs, loading one into run, or deleting
 * one.
 *
 * @return whether allocation succeeded.
 */
bool
rl_alloc_save_files_screen(struct rl_screen* screen,
                           SDL_FRect const* bounds,
                           struct gfx_tileset const* font,
                           struct rl_run* run);

#endif // GINC_ROGUELIKE_SAVE_FILES_SCREEN_H
