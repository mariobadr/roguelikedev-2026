/**
 * @file fov.h
 */
#ifndef GINC_ROGUELIKE_FOV_H
#define GINC_ROGUELIKE_FOV_H

#include <SDL3/SDL_rect.h>

// forward declarations
struct rl_tile_map;

/**
 * The field-of-view for an entity.
 */
struct rl_fov
{
  /** The farthest out the entity can see. */
  int radius;
  /** Cells the entity can see */
  bool* visible;
  /** Cells the entity has seen before */
  bool* explored; // TODO: this shouldn't be here when we have multiple levels
  /** The total cell count (size of visible and explored). */
  int cell_count;
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
