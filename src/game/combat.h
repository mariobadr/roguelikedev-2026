/**
 * @file combat.h
 */
#ifndef GINC_ROGUELIKE_COMBAT_H
#define GINC_ROGUELIKE_COMBAT_H

#include <SDL3/SDL_stdinc.h>

#include "game/event.h"
#include "game/handles.h"

// forward declarations
struct rl_world;
struct rl_actor;
struct rand_state;

/**
 * Resolve an attack from an attacker against a defender.
 *
 * Appends the resulting attack (and, possibly, death) events.
 *
 * @return the damage dealt, or -1 on a miss.
 */
int
rl_resolve_attack(struct rl_actor const* attacker,
                  struct rl_actor* defender,
                  int power,
                  alist(rl_event)* events,
                  struct rand_state* rng);

/**
 * Try a melee attack between two actors in world.
 *
 * @return whether an attack was performed (still true on miss).
 */
bool
rl_attack_melee(struct rl_world* world,
                handle(rl_actor) attacker_handle,
                handle(rl_actor) defender_handle,
                alist(rl_event)* events,
                struct rand_state* rng);

#endif // GINC_ROGUELIKE_COMBAT_H
