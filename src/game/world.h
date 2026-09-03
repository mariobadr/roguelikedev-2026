/**
 * @file world.h
 */
#ifndef GINC_ROGUELIKE_WORLD_H
#define GINC_ROGUELIKE_WORLD_H

#include "container/alist.h"
#include "container/array.h"
#include "container/grid.h"
#include "procgen/layout.h"

#include "game/actor.h"
#include "game/command.h"
#include "game/event.h"
#include "game/item.h"
#include "game/tile_map.h"

// forward declarations
struct rand_state;
struct rl_fov;

/**
 * The identifier for the rogue player in any world.
 */
#define RL_ROGUE_ID 0

/**
 * An growable array of actors.
 */
alist_define_as(struct rl_actor, rl_actor);

/**
 * An growable array of items.
 */
alist_define_as(struct rl_item, rl_item);

/**
 * The game world.
 */
struct rl_world
{
  /** The layout of the level (currently only one level). */
  struct rl_layout layout;
  /** A map of the current level. */
  grid(rl_tile) map;
  /** Next available actor identifier. */
  int next_actor_id;
  /** All actors, including the rogue. */
  alist(rl_actor) actors;
  /** All items. */
  alist(rl_item) items;
  /** A map of distances to reach the player. */
  grid(int) distances;
};

bool
rl_init_world(struct rl_world* world,
              int width,
              int height,
              struct rand_state* rng);

void
rl_free_world(struct rl_world* world);

/**
 * @return the actor corresponding to the given ID (NULL if not found)
 */
struct rl_actor*
rl_get_actor(struct rl_world const* world, int id);

/**
 * @return the item corresponding to the given ID (NULL if not found)
 */
struct rl_item*
rl_get_item(struct rl_world const* world, int id);

/**
 * @return the (alive) actor at position, or NULL if no actor was found.
 */
struct rl_actor*
rl_find_actor(struct rl_world const* world, SDL_Point position);

/**
 * @return the newly added actor of the given type.
 */
struct rl_actor*
rl_add_actor(struct rl_world* world, enum rl_actor_type type);

bool
rl_apply_command(struct rl_world* world,
                 struct rl_command const* player_command,
                 alist(rl_event) * events,
                 struct rand_state* rng);

void
rl_update_actors(struct rl_world* world,
                 struct rl_fov const* fov,
                 alist(rl_event) * events,
                 struct rand_state* rng);

struct rl_actor*
rl_get_actor(struct rl_world const* world, int id);

#endif // GINC_ROGUELIKE_WORLD_H
