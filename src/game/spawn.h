/**
 * @file spawn.h
 */
#ifndef GINC_ROGUELIKE_SPAWN_H
#define GINC_ROGUELIKE_SPAWN_H

#include <SDL3/SDL_stdinc.h>

#include "game/actor.h"
#include "game/item.h"

// forward declarations
struct rand_state;

/**
 * @return how many actors should populate a level at depth.
 */
int
rl_gen_total_actors(int depth, struct rand_state* rng);

/**
 * @return an actor type appropriate for depth.
 */
enum rl_actor_type
rl_gen_actor_type(int depth, struct rand_state* rng);

/**
 * @return how many items should populate a level.
 */
int
rl_gen_total_items(struct rand_state* rng);

/**
 * @return an item type appropriate for depth.
 */
enum rl_item_type
rl_gen_item_type(int depth, struct rand_state* rng);

#endif // GINC_ROGUELIKE_SPAWN_H