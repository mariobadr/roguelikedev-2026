/**
 * @file targeting.h
 */
#ifndef GINC_ROGUELIKE_TARGETING_H
#define GINC_ROGUELIKE_TARGETING_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "container/grid.h"

// forward declarations
struct rl_fov;
struct rl_item_def;
struct rl_world;

/**
 * @return whether dst is an acceptable centre for the tile-targeted item def,
 * as seen through the fov.
 */
bool
rl_is_valid_item_target(struct rl_item_def const* def,
                        struct rl_world const* world,
                        struct rl_fov const* fov,
                        SDL_Point dst);

/**
 * @return whether def, used on centre, affects the world tile p.
 */
bool
rl_item_affects_tile(struct rl_item_def const* def,
                     struct rl_world const* world,
                     SDL_Point centre,
                     SDL_Point p);

SDL_Rect
rl_item_area_bounds(struct rl_item_def const* def, SDL_Point centre);

void
rl_fill_item_area(struct rl_item_def const* def,
                  struct rl_world const* world,
                  SDL_Point centre,
                  grid(boolean)* mask);

#endif // GINC_ROGUELIKE_TARGETING_H
