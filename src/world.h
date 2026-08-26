/**
 * @file world.h
 */
#ifndef GINC_ROGUELIKE_WORLD_H
#define GINC_ROGUELIKE_WORLD_H

#include "alist.h"
#include "command.h"
#include "entity.h"
#include "layout.h"
#include "tile_map.h"

// forward declarations
struct rand_state;

alist_define_as(struct rl_entity, rl_entity);

struct rl_world
{
  /** The layout of the level (currently only one level). */
  struct rl_layout layout;
  /** A map of the current level. */
  struct rl_tile_map map;
  /** The player entity */
  struct rl_entity rogue;
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
                struct rand_state* rng);

#endif // GINC_ROGUELIKE_WORLD_H
