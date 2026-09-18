/**
 * @file main_menu.h
 */
#ifndef GINC_ROGUELIKE_MAIN_MENU_SCREEN_H
#define GINC_ROGUELIKE_MAIN_MENU_SCREEN_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// forward declarations
struct gfx_tileset;
struct rl_run;
struct rl_screen;

bool
rl_alloc_main_menu_screen(struct rl_screen* screen,
                          SDL_FRect const* bounds,
                          struct gfx_tileset const* font,
                          struct rl_run* run);

#endif // GINC_ROGUELIKE_MAIN_MENU_SCREEN_H
