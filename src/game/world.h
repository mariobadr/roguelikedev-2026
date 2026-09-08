/**
 * @file world.h
 */
#ifndef GINC_ROGUELIKE_WORLD_H
#define GINC_ROGUELIKE_WORLD_H

#include "container/alist.h"

#include "game/actor.h"
#include "game/item.h"
#include "game/level.h"

// forward declarations
struct rand_state;

/**
 * The identifier for the rogue player in any world.
 */
#define RL_ROGUE_ID 0

/**
 * The game world.
 */
struct rl_world
{
  /** The current level (currently only one level). */
  struct rl_level level;
  /** The player. */
  struct rl_actor rogue;
  /** All actors except the rogue. */
  alist(rl_actor) actors;
  /** All items. */
  alist(rl_item) items;
};

/**
 * Allocate a new world.
 */
bool
rl_alloc_world(struct rl_world* world, int width, int height);

/**
 * Free the resources used by world.
 */
void
rl_free_world(struct rl_world* world);

/**
 * @return the actor corresponding to the given ID (NULL if not found)
 */
struct rl_actor const*
rl_get_actor(struct rl_world const* world, int id);

/**
 * @return the actor for modification (NULL if not found).
 */
struct rl_actor*
rl_edit_actor(struct rl_world* world, int id);

/**
 * @return the total number of actors in the world, including the rogue.
 */
int
rl_actor_count(struct rl_world const* world);

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

#endif // GINC_ROGUELIKE_WORLD_H
