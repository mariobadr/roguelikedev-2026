/**
 * @file world.h
 */
#ifndef GINC_ROGUELIKE_WORLD_H
#define GINC_ROGUELIKE_WORLD_H

#include "command.h"
#include "entity.h"
#include "layout.h"
#include "tile_map.h"

// forward declarations
struct rand_state;

struct rl_world
{
  /** The layout of the level (currently only one level). */
  struct rl_layout layout;
  /** A map of the current level. */
  struct rl_tile_map map;
  /** The player entity */
  struct rl_entity rogue;
  /** All other entities */
  struct rl_entity* entities;
  /** The number of other entities */
  int entity_count;
};

bool
rl_init_world(struct rl_world* world,
              int width,
              int height,
              struct rand_state* rng);

void
rl_free_world(struct rl_world* world);

void
rl_update_world(struct rl_world* world,
                struct rl_command const* player_command);

#endif // GINC_ROGUELIKE_WORLD_H
