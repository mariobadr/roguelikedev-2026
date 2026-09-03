/**
 * @file world.h
 */
#ifndef GINC_ROGUELIKE_WORLD_H
#define GINC_ROGUELIKE_WORLD_H

#include "container/alist.h"
#include "container/array.h"
#include "container/grid.h"
#include "procgen/layout.h"

#include "actor.h"
#include "command.h"
#include "event.h"
#include "tile_map.h"

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
  /** All other actors */
  alist(rl_actor) actors;
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
