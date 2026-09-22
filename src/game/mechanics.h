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
struct rl_actor;
struct rl_world;
struct rand_state;

/**
 * Add experience points to the player, fully healing them on a level up.
 *
 * @return the number of levels gained.
 */
int
rl_gain_xp(struct rl_world* world, int amount, alist(rl_event)* events);

/**
 * Try to move the actor in world to dst.
 *
 * @return whether world was updated (i.e., move succeeded).
 */
bool
rl_move(struct rl_world* world, handle(rl_actor) actor_handle, SDL_Point dst);

/**
 * Resolve a melee attack, recording the attack (and, possibly, death) in
 * events.
 *
 * @param attacker must be alive.
 * @param defender must be alive.
 */
void
rl_attack_melee(struct rl_world const* world,
                struct rl_actor const* attacker,
                struct rl_actor* defender,
                alist(rl_event)* events,
                struct rand_state* rng);

/**
 * Resolve a magic attack, which ignores armour and cannot crit, recording the
 * attack (and, possibly, death) in events.
 *
 * @param defender must be alive.
 */
void
rl_attack_magic(struct rl_actor const* attacker,
                struct rl_actor* defender,
                int power,
                alist(rl_event)* events,
                struct rand_state* rng);

/**
 * Try to pick up an item found at dst.
 *
 * An equippable item is equipped at once, leaving the item it replaces at dst.
 *
 * @return whether an item was picked up.
 */
bool
rl_pick_up_item(struct rl_world* world,
                handle(rl_actor) actor_handle,
                SDL_Point dst,
                alist(rl_event)* events);

/**
 * Roll for at most one item and drop it where the actor died.
 */
void
rl_drop_loot(struct rl_world* world,
             handle(rl_actor) actor_handle,
             alist(rl_event)* events,
             struct rand_state* rng);

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
            alist(rl_event)* events,
            struct rand_state* rng);

/**
 * Ascending above the first level is not allowed.
 *
 * @param actor must be alive.
 *
 * @return whether the rogue can take the stairs at its current position.
 */
bool
rl_can_take_stairs(struct rl_world const* world, struct rl_actor const* actor);

/**
 * Try to take the stairs.
 *
 * Descending below the deepest level generates a new one.
 *
 * Note: Only the rogue can take stairs (for now?)
 *
 * @return whether the actor changed levels.
 */
bool
rl_take_stairs(struct rl_world* world,
               handle(rl_actor) actor_handle,
               alist(rl_event)* events,
               struct rand_state* rng);

#endif // GINC_ROGUELIKE_MECHANICS_H
