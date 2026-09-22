#include "targeting.h"

#include <SDL3/SDL_assert.h>

#include "spatial/fov.h"

#include "item_def.h"
#include "world.h"

handle(rl_actor)
rl_find_nearest_visible_actor(struct rl_world const* world,
                              handle(rl_actor) attacker)
{
  handle(rl_actor) nearest = handle_invalid(rl_actor);
  int nearest_dist_sq = 0;

  struct rl_actor const* self = rl_borrow_actor(world, attacker);
  if (self == NULL) {
    return nearest;
  }

  struct rl_level const* level = rl_get_current_level(world);
  for (size_t i = 0; i < alist_len(&level->actors); i++) {
    struct rl_actor const* candidate =
      rl_borrow_actor(world, *alist_at(&level->actors, i));
    if (handle_equal(candidate->handle, attacker)) {
      continue;
    }

    if (!rl_actor_is_alive(candidate)) {
      continue;
    }

    if (!sptl_is_tile_visible(&world->player.fov, candidate->pos)) {
      continue;
    }

    int const dx = candidate->pos.x - self->pos.x;
    int const dy = candidate->pos.y - self->pos.y;
    int const dist_sq = dx * dx + dy * dy;

    if (handle_is_nonnull(nearest) && dist_sq >= nearest_dist_sq) {
      continue;
    }

    nearest = candidate->handle;
    nearest_dist_sq = dist_sq;
  }

  return nearest;
}

bool
rl_is_valid_item_target(struct rl_item_consumable_def const* item,
                        struct rl_world const* world,
                        SDL_Point dst)
{
  (void)item;

  struct rl_level const* level = rl_get_current_level(world);
  if (!grid_contains(&level->map, dst.x, dst.y)) {
    return false;
  }

  return sptl_is_tile_visible(&world->player.fov, dst);
}

bool
rl_item_affects_tile(struct rl_item_consumable_def const* item,
                     struct rl_world const* world,
                     SDL_Point centre,
                     SDL_Point p)
{
  int const radius = item->area_radius;
  int const dx = p.x - centre.x;
  int const dy = p.y - centre.y;
  if (dx * dx + dy * dy > radius * radius) {
    // outside the blast (Euclidean)
    return false;
  }

  grid(rl_tile) const* map = &rl_get_current_level(world)->map;
  return grid_contains(map, p.x, p.y);
}

SDL_Rect
rl_item_area_bounds(struct rl_item_consumable_def const* item, SDL_Point centre)
{
  int const radius = item->area_radius;
  int const extent = 2 * radius + 1;

  SDL_Rect bounds = { 0 };
  bounds.x = centre.x - radius;
  bounds.y = centre.y - radius;
  bounds.w = extent;
  bounds.h = extent;

  return bounds;
}

void
rl_fill_item_area(struct rl_item_consumable_def const* item,
                  struct rl_world const* world,
                  SDL_Point centre,
                  grid(boolean) * mask)
{
  SDL_Rect const bounds = rl_item_area_bounds(item, centre);
  SDL_assert(grid_width(mask) == bounds.w);
  SDL_assert(grid_height(mask) == bounds.h);

  for (int y = 0; y < bounds.h; y++) {
    for (int x = 0; x < bounds.w; x++) {
      SDL_Point const p = { bounds.x + x, bounds.y + y };
      *grid_at(mask, x, y) = rl_item_affects_tile(item, world, centre, p);
    }
  }
}
