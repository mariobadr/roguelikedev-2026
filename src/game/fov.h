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
  /** Cells the actor can see. */
  grid(boolean) visible;
};

/**
 * Allocate a field of view for a map of width by height cells.
 *
 * @return whether allocation succeeded.
 */
bool
rl_alloc_fov(struct rl_fov* fov, int width, int height, int radius);

/**
 * Free the field of view.
 */
void
rl_free_fov(struct rl_fov* fov);

/**
 * Make every cell not visible.
 */
void
rl_clear_fov(struct rl_fov* fov);

/**
 * Update the visible cells for an actor at origin on map.
 */
void
rl_update_fov(struct rl_fov* fov, grid(rl_tile) const* map, SDL_Point origin);

/**
 * @return whether p is visible.
 */
static inline bool
rl_is_tile_visible(struct rl_fov const* fov, SDL_Point p)
{
  if (!grid_contains(&fov->visible, p.x, p.y)) {
    return false;
  }

  return *grid_at(&fov->visible, p.x, p.y);
}

#endif // GINC_ROGUELIKE_FOV_H
