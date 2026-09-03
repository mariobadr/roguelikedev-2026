/**
 * @file fov.h
 */
#ifndef GINC_ROGUELIKE_FOV_H
#define GINC_ROGUELIKE_FOV_H

#include <SDL3/SDL_rect.h>

#include "container/array.h"

// forward declarations
struct rl_tile_map;

/**
 * The field-of-view for an actor.
 */
struct rl_fov
{
  /** The centre of the field (circle). */
  SDL_Point origin;
  /** The farthest out the actor can see. */
  int radius;
  /** Cells the actor can see */
  array(boolean) visible;
  /** Cells the actor has seen before */
  array(boolean) explored; // TODO: this shouldn't be here when we have multiple levels
};

bool
rl_init_fov(struct rl_fov* fov, int cell_count, int radius);

void
rl_free_fov(struct rl_fov* fov);

void
rl_clear_fov(struct rl_fov* fov);

void
rl_update_fov(struct rl_fov* fov,
              struct rl_tile_map const* map,
              SDL_Point origin);

#endif // GINC_ROGUELIKE_FOV_H
