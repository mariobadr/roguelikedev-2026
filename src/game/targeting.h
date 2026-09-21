/**
 * @file targeting.h
 */
#ifndef GINC_ROGUELIKE_TARGETING_H
#define GINC_ROGUELIKE_TARGETING_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "container/grid.h"

#include "game/handles.h"

// forward declarations
struct rl_item_consumable_def;
struct rl_world;

/**
 * @return the nearest (visible) actor to attacker, or an invalid handle if none
 * exists.
 */
handle(rl_actor)
rl_find_nearest_visible_actor(struct rl_world const* world,
                              handle(rl_actor) attacker);

/**
 * @return whether dst is an acceptable centre for the tile-targeted item, as
 * seen through the player's field-of-view.
 */
bool
rl_is_valid_item_target(struct rl_item_consumable_def const* item,
                        struct rl_world const* world,
                        SDL_Point dst);

/**
 * @return whether item, used on centre, affects the world tile p.
 */
bool
rl_item_affects_tile(struct rl_item_consumable_def const* item,
                     struct rl_world const* world,
                     SDL_Point centre,
                     SDL_Point p);

/**
 * @return the region that item, used on centre, can affect.
 */
SDL_Rect
rl_item_area_bounds(struct rl_item_consumable_def const* item,
                    SDL_Point centre);

/**
 * Mark the tiles that item, used on centre, affects.
 *
 * @param mask one entry per tile of the item's area bounds.
 */
void
rl_fill_item_area(struct rl_item_consumable_def const* item,
                  struct rl_world const* world,
                  SDL_Point centre,
                  grid(boolean) * mask);

#endif // GINC_ROGUELIKE_TARGETING_H
