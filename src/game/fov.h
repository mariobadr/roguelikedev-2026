/**
 * @file fov.h
 */
#ifndef GINC_ROGUELIKE_FOV_H
#define GINC_ROGUELIKE_FOV_H

#include <SDL3/SDL_rect.h>

#include "container/grid.h"

#include "game/tile.h"

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
  grid(boolean) visible;
};

bool
rl_alloc_fov(struct rl_fov* fov, int width, int height, int radius);

void
rl_free_fov(struct rl_fov* fov);

void
rl_clear_fov(struct rl_fov* fov);

void
rl_update_fov(struct rl_fov* fov, grid(rl_tile) const* map, SDL_Point origin);

#endif // GINC_ROGUELIKE_FOV_H
