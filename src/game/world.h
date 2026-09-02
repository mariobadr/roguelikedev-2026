/**
 * @file world.h
 */
#ifndef GINC_ROGUELIKE_WORLD_H
#define GINC_ROGUELIKE_WORLD_H

#include "container/alist.h"
#include "command.h"
#include "entity.h"
#include "event.h"
#include "procgen/layout.h"
#include "tile_map.h"

// forward declarations
struct rand_state;

/**
 * The identifier for the rogue player in any world.
 */
#define RL_ROGUE_ID 0

/**
 * An growable array of entities.
 */
alist_define_as(struct rl_entity, rl_entity);

/**
 * The game world.
 */
struct rl_world
{
  /** The layout of the level (currently only one level). */
  struct rl_layout layout;
  /** A map of the current level. */
  struct rl_tile_map map;
  /** Next available entity identifier. */
  int next_entity_id;
  /** All other entities */
  alist(rl_entity) entities;
};

bool
rl_init_world(struct rl_world* world,
              int width,
              int height,
              struct rand_state* rng);

void
rl_free_world(struct rl_world* world);

bool
rl_update_world(struct rl_world* world,
                struct rl_command const* player_command,
                alist(rl_event) * events,
                struct rand_state* rng);

struct rl_entity*
rl_get_entity(struct rl_world const* world, int id);

#endif // GINC_ROGUELIKE_WORLD_H
