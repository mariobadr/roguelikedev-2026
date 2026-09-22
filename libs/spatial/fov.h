/**
 * @file fov.h
 */
#ifndef GINC_SPATIAL_FOV_H
#define GINC_SPATIAL_FOV_H

#include <SDL3/SDL_rect.h>

#include "container/grid.h"

/**
 * The search runs outwards from the origin, so it is only called for tiles
 * inside the field.
 *
 * @return whether an actor can see through the tile at p.
 */
typedef bool (*sptl_transparent_fn)(void* context, SDL_Point p);

/**
 * The field-of-view for an actor.
 *
 * @invariant visible holds the field computed from origin.
 */
struct sptl_fov
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
sptl_alloc_fov(struct sptl_fov* fov, int width, int height, int radius);

/**
 * Free the field of view.
 */
void
sptl_free_fov(struct sptl_fov* fov);

/**
 * Update the visible cells for an actor at origin.
 *
 * @param origin must be inside the field.
 */
void
sptl_update_fov(struct sptl_fov* fov,
                SDL_Point origin,
                sptl_transparent_fn is_transparent,
                void* context);

/**
 * @return whether p is visible.
 */
static inline bool
sptl_is_tile_visible(struct sptl_fov const* fov, SDL_Point p)
{
  if (!grid_contains(&fov->visible, p.x, p.y)) {
    return false;
  }

  return *grid_at(&fov->visible, p.x, p.y);
}

#endif // GINC_SPATIAL_FOV_H
