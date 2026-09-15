/**
 * @file spawn.h
 */
#ifndef GINC_ROGUELIKE_SPAWN_H
#define GINC_ROGUELIKE_SPAWN_H

#include <SDL3/SDL_stdinc.h>

#include "game/item.h"

// forward declarations
struct rand_state;
struct rl_level;
struct rl_layout;
struct rl_world;

bool
rl_spawn_actors(struct rl_level const* level,
                struct rl_layout const* layout,
                struct rl_world* world,
                int reserved_room,
                struct rand_state* rng);

bool
rl_spawn_items(struct rl_level const* level,
               struct rl_layout const* layout,
               alist(rl_item) * items,
               struct rand_state* rng);

#endif // GINC_ROGUELIKE_SPAWN_H
