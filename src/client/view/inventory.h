/**
 * @file inventory.h
 */
#ifndef GINC_ROGUELIKE_INVENTORY_VIEW_H
#define GINC_ROGUELIKE_INVENTORY_VIEW_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// forward declarations
struct rl_view;
struct rl_world;

bool
rl_alloc_inv_view(struct rl_view* view,
                  struct rl_world const* world,
                  SDL_FRect const* viewport,
                  float line_height);

#endif // GINC_ROGUELIKE_INVENTORY_VIEW_H
