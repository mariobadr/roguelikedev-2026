/**
 * @file world.h
 */
#ifndef GINC_ROGUELIKE_WORLD_H
#define GINC_ROGUELIKE_WORLD_H

#include "container/alist.h"

#include "game/actor.h"
#include "game/handles.h"
#include "game/item.h"
#include "game/level.h"

// forward declarations
struct rand_state;

/**
 * The game world.
 */
struct rl_world
{
  /** The levels visited so far. */
  alist(rl_level) levels;
  /** All actors, including the rogue. */
  alist(rl_actor) actors;
  /** All items. */
  alist(rl_item) items;
  /** The rogue (player). Invalid until a new game creates it. */
  handle(rl_actor) rogue;
};

/**
 * Allocate a new world.
 */
bool
rl_alloc_world(struct rl_world* world);

/**
 * Free the resources used by world.
 */
void
rl_free_world(struct rl_world* world);

/**
 * Add a new actor of the given type to the world and assign its handle.
 *
 * Warning: the pointer is only valid until the next actor is added.
 *
 * @return the new actor, or NULL if allocation failed.
 */
struct rl_actor*
rl_add_actor(struct rl_world* world, enum rl_actor_type type);

/**
 * @return the rogue's handle (invalid until a new game creates the rogue).
 */
handle(rl_actor) rl_rogue_handle(struct rl_world const* world);

/**
 * @return the actor referred to by actor_handle (NULL if not found)
 */
struct rl_actor const*
rl_get_actor(struct rl_world const* world, handle(rl_actor) actor_handle);

/**
 * @return the actor for modification (NULL if not found).
 */
struct rl_actor*
rl_edit_actor(struct rl_world* world, handle(rl_actor) actor_handle);

/**
 * @return the number of actor indices; valid indices for rl_get_actor_at and
 * rl_edit_actor_at are in [0, count).
 */
int
rl_actor_count(struct rl_world const* world);

/**
 * @return the actor at index, or NULL if there is no actor there.
 */
struct rl_actor const*
rl_get_actor_at(struct rl_world const* world, int index);

/**
 * @return the actor at index for modification, or NULL if there is no actor
 * there.
 */
struct rl_actor*
rl_edit_actor_at(struct rl_world* world, int index);

/**
 * @return the (alive) actor at position, or NULL if no actor was found.
 */
struct rl_actor const*
rl_find_actor(struct rl_world const* world, SDL_Point position);

/**
 * @return the item corresponding to the given ID (NULL if not found)
 */
struct rl_item const*
rl_get_item(struct rl_world const* world, int id);

/**
 * @return the item for modification (NULL if not found).
 */
struct rl_item*
rl_edit_item(struct rl_world* world, int id);

/**
 * @return the item at the position, or NULL if no item was found.
 */
struct rl_item*
rl_find_item(struct rl_world* world, SDL_Point position);

/**
 * @return the level the rogue is currently on.
 */
struct rl_level const*
rl_get_current_level(struct rl_world const* world);

/**
 * @return the level (for modification) the rogue is currently on.
 */
struct rl_level*
rl_edit_current_level(struct rl_world* world);

#endif // GINC_ROGUELIKE_WORLD_H
