/**
 * @file world.h
 */
#ifndef GINC_ROGUELIKE_WORLD_H
#define GINC_ROGUELIKE_WORLD_H

#include "layout.h"
#include "tile_map.h"

// forward declarations
struct rand_state;

struct rl_world
{
  struct rl_layout layout;
  struct rl_tile_map map;
};

bool
rl_init_world(struct rl_world* world,
              int width,
              int height,
              struct rand_state* rng);

void
rl_free_world(struct rl_world* world);

#endif // GINC_ROGUELIKE_WORLD_H
