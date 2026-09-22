/**
 * @file world.h
 */
#ifndef GINC_ROGUELIKE_WORLD_H
#define GINC_ROGUELIKE_WORLD_H

#include "container/alist.h"
#include "container/pool.h"

#include "game/actor.h"
#include "game/actor_def.h"
#include "game/handles.h"
#include "game/item.h"
#include "game/item_def.h"
#include "game/level.h"
#include "game/player.h"

/**
 * A pool of actors, addressed by handle(rl_actor).
 */
pool_define_as(struct rl_actor, rl_actor);

/**
 * A pool of items, addressed by handle(rl_item).
 */
pool_define_as(struct rl_item, rl_item);

/**
 * The game world.
 *
 * @invariant current_level is a valid index into levels.
 * @invariant every level's map, and player.scent.distances, have the same
 * shape.
 * @invariant player.actor is live.
 */
struct rl_world
{
  /** The levels visited so far. */
  alist(rl_level) levels;
  /** Index of the current level. */
  int current_level;
  /** All actors, including the rogue. */
  pool(rl_actor) actors;
  /** All items, wherever they are. */
  pool(rl_item) items;
  /** Player state. */
  struct rl_player player;
};

/**
 * Allocate a new world.
 *
 * @return whether allocation succeeded.
 */
bool
rl_alloc_world(struct rl_world* world);

/**
 * Allocate player runtime buffers and initialise visibility.
 *
 * @return whether allocation succeeded.
 */
bool
rl_create_player(struct rl_world* world);

/**
 * Update the player's field-of-view and explored tiles on the current level.
 */
void
rl_update_visibility(struct rl_world* world);

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
 * Add a new actor of the given type and level to the world.
 *
 * @param level must be at least 1.
 *
 * @return the new actor's handle, or an invalid handle if allocation failed.
 */
handle(rl_actor)
rl_create_actor(struct rl_world* world, enum rl_actor_type type, int level);

/**
 * @return the handle of the (alive) actor at position on level, or an
 * invalid handle if no actor was found.
 */
handle(rl_actor)
rl_find_actor(struct rl_world const* world,
              struct rl_level const* level,
              SDL_Point position);

/**
 * @return the actor referred to by actor_handle (NULL if not found).
 */
struct rl_actor const*
rl_borrow_actor(struct rl_world const* world, handle(rl_actor) actor_handle);

/**
 * @return the actor for modification (NULL if not found).
 */
struct rl_actor*
rl_borrow_mut_actor(struct rl_world* world, handle(rl_actor) actor_handle);

/**
 * Add a new, unplaced item of the given type to the world, at the tier that
 * level reaches.
 *
 * @param level must be at least 1.
 *
 * @return the new item's handle, or an invalid handle if allocation failed.
 */
handle(rl_item)
rl_create_item(struct rl_world* world, enum rl_item_type type, int level);

/**
 * Create an item of the given type on level's floor at pos, at the tier that
 * item_level reaches.
 *
 * @param item_level must be at least 1.
 *
 * @return the new item's handle, or an invalid handle if the item could not be
 * created or added to level.
 */
handle(rl_item)
rl_add_item_to_level(struct rl_world* world,
                     struct rl_level* level,
                     enum rl_item_type type,
                     int item_level,
                     SDL_Point pos);

/**
 * @return the handle of an item on level's floor at position, or an invalid
 * handle if no item was found.
 */
handle(rl_item)
rl_find_item(struct rl_world const* world,
             struct rl_level const* level,
             SDL_Point position);

/**
 * @return the handle of the nth item holder is holding, or an invalid handle
 * if n is out of range.
 */
handle(rl_item)
rl_find_held_item(struct rl_world const* world, handle(rl_actor) holder, int n);

/**
 * @return how many items holder is holding.
 */
int
rl_count_held_items(struct rl_world const* world, handle(rl_actor) holder);

/**
 * @return the item referred to by item_handle (NULL if not found).
 */
struct rl_item const*
rl_borrow_item(struct rl_world const* world, handle(rl_item) item_handle);

/**
 * @return the item for modification (NULL if not found).
 */
struct rl_item*
rl_borrow_mut_item(struct rl_world* world, handle(rl_item) item_handle);

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
