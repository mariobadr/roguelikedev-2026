/**
 * @file character.h
 */
#ifndef GINC_ROGUELIKE_CHARACTER_VIEW_H
#define GINC_ROGUELIKE_CHARACTER_VIEW_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// forward declarations
struct gfx_tileset;
struct rl_view;
struct rl_world;

/**
 * Allocate a character view.
 *
 * @return whether allocation succeeded.
 */
bool
rl_alloc_character_view(struct rl_view* view,
                        struct rl_world const* world,
                        SDL_FRect const* viewport,
                        struct gfx_tileset const* font);

#endif // GINC_ROGUELIKE_CHARACTER_VIEW_H
