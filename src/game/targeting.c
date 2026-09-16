#include "targeting.h"

#include <SDL3/SDL_assert.h>

#include "fov.h"
#include "item.h"
#include "world.h"

bool
rl_is_valid_item_target(struct rl_item_def const* def,
                        struct rl_world const* world,
                        struct rl_fov const* fov,
                        SDL_Point dst)
{
  (void)def;

  struct rl_level const* level = rl_get_current_level(world);
  if (!grid_contains(&level->map, dst.x, dst.y)) {
    return false;
  }

  return rl_is_tile_visible(fov, dst);
}

bool
rl_item_affects_tile(struct rl_item_def const* def,
                     struct rl_world const* world,
                     SDL_Point centre,
                     SDL_Point p)
{
  int const radius = def->area_radius;
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
rl_item_area_bounds(struct rl_item_def const* def, SDL_Point centre)
{
  int const radius = def->area_radius;
  int const extent = 2 * radius + 1;

  SDL_Rect bounds = { 0 };
  bounds.x = centre.x - radius;
  bounds.y = centre.y - radius;
  bounds.w = extent;
  bounds.h = extent;

  return bounds;
}

void
rl_fill_item_area(struct rl_item_def const* def,
                  struct rl_world const* world,
                  SDL_Point centre,
                  grid(boolean)* mask)
{
  SDL_Rect const bounds = rl_item_area_bounds(def, centre);
  SDL_assert(grid_width(mask) == bounds.w);
  SDL_assert(grid_height(mask) == bounds.h);

  for (int y = 0; y < bounds.h; y++) {
    for (int x = 0; x < bounds.w; x++) {
      SDL_Point const p = { bounds.x + x, bounds.y + y };
      *grid_at(mask, x, y) = rl_item_affects_tile(def, world, centre, p);
    }
  }
}
