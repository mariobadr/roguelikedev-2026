/**
 * @file spawn.h
 */
#ifndef GINC_ROGUELIKE_SPAWN_H
#define GINC_ROGUELIKE_SPAWN_H

#include <SDL3/SDL_stdinc.h>

// forward declarations
struct rand_state;
struct rl_level;
struct rl_layout;
struct rl_world;

/**
 * Populate unoccupied room tiles, excluding reserved_room.
 *
 * @return whether spawning succeeded. Running out of space is not a failure.
 */
bool
rl_spawn_actors(struct rl_level* level,
                struct rl_layout const* layout,
                struct rl_world* world,
                int reserved_room,
                struct rand_state* rng);

/**
 * Populate unoccupied room tiles with items.
 *
 * @return whether spawning succeeded. Running out of space is not a failure.
 */
bool
rl_spawn_items(struct rl_level* level,
               struct rl_layout const* layout,
               struct rl_world* world,
               struct rand_state* rng);

#endif // GINC_ROGUELIKE_SPAWN_H
