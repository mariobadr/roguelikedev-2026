/**
 * @file world.h
 */
#ifndef GINC_ROGUELIKE_WORLD_H
#define GINC_ROGUELIKE_WORLD_H

#include "container/alist.h"
#include "container/pool.h"

#include "game/actor.h"
#include "game/handles.h"
#include "game/item.h"
#include "game/level.h"

// forward declarations
struct rand_state;

/**
 * A pool of actors, addressed by handle(rl_actor).
 */
pool_define_as(struct rl_actor, rl_actor);

/**
 * The game world.
 */
struct rl_world
{
  /** The levels visited so far. */
  alist(rl_level) levels;
  /**
   * Index into levels of the level the rogue is on; -1 until a new game
   * creates one.
   */
  int current_level;
  /** All actors, including the rogue. */
  pool(rl_actor) actors;
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
 * @return the rogue's handle.
 */
handle(rl_actor)
rl_get_rogue(struct rl_world const* world);

/**
 * Add a new actor of the given type to the world.
 *
 * @return the new actor's handle, or an invalid handle if allocation failed.
 */
handle(rl_actor)
rl_create_actor(struct rl_world* world, enum rl_actor_type type);

/**
 * @return the handle of the (alive) actor at position on level, or an
 * invalid handle if no actor was found.
 */
handle(rl_actor)
rl_find_actor(struct rl_world const* world,
              struct rl_level const* level,
              SDL_Point position);

/**
 * @return the actor referred to by actor_handle (NULL if not found)
 */
struct rl_actor const*
rl_borrow_actor(struct rl_world const* world, handle(rl_actor) actor_handle);

/**
 * @return the actor for modification (NULL if not found).
 */
struct rl_actor*
rl_borrow_mut_actor(struct rl_world* world, handle(rl_actor) actor_handle);

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
