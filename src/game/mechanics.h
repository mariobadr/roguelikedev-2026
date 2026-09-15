/**
 * @file mechanics.h
 */
#ifndef GINC_ROGUELIKE_MECHANICS_H
#define GINC_ROGUELIKE_MECHANICS_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "game/event.h"
#include "game/handles.h"

// forward declarations
struct rl_world;
struct rl_fov;
struct rand_state;

/** Radius, in tiles, of a damage-area item's blast (Euclidean). */
#define RL_DAMAGE_AREA_RADIUS 3

/**
 * Try to move the actor in world to dst.
 *
 * @return whether world was updated (i.e., move succeeded).
 */
bool
rl_move(struct rl_world* world, handle(rl_actor) actor_handle, SDL_Point dst);

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

/**
 * Try to pick up an item found at dst.
 *
 * @return whether an item was picked up.
 */
bool
rl_pick_up_item(struct rl_world* world,
                handle(rl_actor) actor_handle,
                SDL_Point dst,
                alist(rl_event)* events);

/**
 * Try to use an item.
 *
 * @return whether the item was used successfully.
 */
bool
rl_use_item(struct rl_world* world,
            handle(rl_actor) actor_handle,
            handle(rl_item) item_handle,
            SDL_Point target,
            struct rl_fov const* fov,
            alist(rl_event)* events,
            struct rand_state* rng);

#endif // GINC_ROGUELIKE_MECHANICS_H
