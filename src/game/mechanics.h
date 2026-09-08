/**
 * @file mechanics.h
 */
#ifndef GINC_ROGUELIKE_MECHANICS_H
#define GINC_ROGUELIKE_MECHANICS_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "game/event.h"

// forward declarations
struct rl_world;
struct rand_state;

/**
 * Try to move the actor in world with actor_id to dst.
 *
 * @return whether world was updated (i.e., move succeeded).
 */
bool
rl_move(struct rl_world* world, int actor_id, SDL_Point dst);

/**
 * Try a melee attack between two actors in world.
 * 
 * @return whether an attack was performed (still true on miss).
 */
bool
rl_attack_melee(struct rl_world* world,
                int attacker_id,
                int defender_id,
                alist(rl_event) * events,
                struct rand_state* rng);

#endif // GINC_ROGUELIKE_MECHANICS_H
