#include "fov.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "tile_map.h"

/**
 * State shared by every scan.
 */
struct fov_context
{
  grid(rl_tile) const* map;
  SDL_Point origin;
  int radius;
};

/**
 * A transformation from coordinates in one octant to map coordinates.
 */
struct octant_transform
{
  int xx;
  int xy;
  int yx;
  int yy;
};

static SDL_Point
transform(struct fov_context const* context,
          struct octant_transform const* octant,
          int dx,
          int dy)
{
  return (SDL_Point){
    .x = context->origin.x + dx * octant->xx + dy * octant->xy,
    .y = context->origin.y + dx * octant->yx + dy * octant->yy,
  };
}

/** Transformations used to scan all eight octants around an origin. */
static struct octant_transform const octants[] = {
  { .xx = 1, .yy = 1 },   // NW to N
  { .xy = 1, .yx = 1 },   // NW to W
  { .xy = -1, .yx = 1 },  // NE to E
  { .xx = -1, .yy = 1 },  // NE to N
  { .xx = -1, .yy = -1 }, // SE to S
  { .xy = -1, .yx = -1 }, // SE to E
  { .xy = 1, .yx = -1 },  // SW to W
  { .xx = 1, .yy = -1 },  // SW to S
};

/**
 * @return whether (dx, dy) are within radius
 */
static bool
is_within_radius(int dx, int dy, int radius)
{
  return dx * dx + dy * dy <= radius * radius;
}

/**
 * This is "recursive shadowcasting", see:
 * https://www.roguebasin.com/index.php/FOV_using_recursive_shadowcasting
 *
 * Fadden's C# implementation was a useful reference:
 * https://fadden.com/tech/ShadowCast.cs.txt
 */
static void
scan_octant(struct fov_context const* context,
            struct octant_transform const* octant,
            int start_depth,
            double start_slope,
            double end_slope,
            bool* visible)
{
  if (start_depth > context->radius) {
    // no rows within the distance limit
    return;
  }

  if (start_slope < end_slope) {
    // nothing to do
    return;
  }

  double saved_right_slope = start_slope;

  for (int depth = start_depth; depth <= context->radius; depth++) {
    bool previous_was_blocking = false;

    int dy = -depth;
    for (int dx = -depth; dx <= 0; dx++) {

      double left_slope = (dx - 0.5f) / (dy + 0.5f);
      double right_slope = (dx + 0.5f) / (dy - 0.5f);

      // Strict comparisons make tiles touching a view boundary at a corner
      // visible.
      if (start_slope < right_slope) {
        // this tile is before the scan's start slope
        continue;
      }

      if (end_slope > left_slope) {
        // remaining tiles are beyond the scan's end slope
        break;
      }

      SDL_Point tile_pos = transform(context, octant, dx, dy);
      bool in_bounds = grid_contains(context->map, tile_pos.x, tile_pos.y);

      enum rl_tile tile = RL_TILE_WALL;
      if (in_bounds) {
        tile = *grid_at(context->map, tile_pos.x, tile_pos.y);

        if (is_within_radius(dx, dy, context->radius)) {
          size_t index = grid_index_of(context->map, tile_pos.x, tile_pos.y);
          visible[index] = true;
        }
      }

      bool is_blocking = !in_bounds || !rl_is_transparent(tile);

      if (previous_was_blocking) {
        if (is_blocking) {
          // we are still inside a section of blocking tiles
          saved_right_slope = right_slope;
        } else {
          // continue after the section of blocking tiles
          previous_was_blocking = false;
          start_slope = saved_right_slope;
        }
      } else if (is_blocking && depth < context->radius) {
        previous_was_blocking = true;

        // recurse at the next depth, ending at the blocking tile's left edge
        scan_octant(
          context, octant, depth + 1, start_slope, left_slope, visible);
        // save where scanning resumes after the wall
        saved_right_slope = right_slope;
      }
    }

    if (previous_was_blocking) {
      // the rest of the view area is blocked
      break;
    }
  }
}

/**
 * Update out with a field of view from origin.
 *
 * @param map       a map to inspect for calculating the field of view
 * @param origin    the centre point
 * @param radius    the radius of the circle around origin
 * @param out       The visibility of the tiles in map
 */
static void
compute_fov(grid(rl_tile) const* map,
            SDL_Point origin,
            int radius,
            grid(boolean) * out)
{
  // set everything to not visible
  SDL_memset(out->data, 0, grid_count(out) * sizeof(*out->data));

  // but, of course, the origin is visible
  size_t index = grid_index_of(map, origin.x, origin.y);
  *grid_at_index(out, index) = true;

  // things that don't change when calling scan_octant
  struct fov_context context = { 0 };
  context.map = map;
  context.origin = origin;
  context.radius = radius;

  for (int octant = 0; octant < 8; octant++) {
    scan_octant(&context, &octants[octant], 1, 1.0, 0.0, out->data);
  }
}

bool
rl_init_fov(struct rl_fov* fov, int width, int height, int radius)
{
  if (!grid_alloc(&fov->visible, width, height)) {
    SDL_Log("grid_alloc failed: %s", SDL_GetError());
    return false;
  }

  if (!grid_alloc(&fov->explored, width, height)) {
    SDL_Log("grid_alloc failed: %s", SDL_GetError());
    rl_free_fov(fov);
    return false;
  }

  fov->origin.x = -1;
  fov->origin.y = -1;
  fov->radius = radius;

  return true;
}

void
rl_free_fov(struct rl_fov* fov)
{
  if (fov == NULL) {
    return;
  }

  grid_free(&fov->visible);
  grid_free(&fov->explored);

  fov->radius = 0;
}

void
rl_clear_fov(struct rl_fov* fov)
{
  // invalidate the origin
  fov->origin.x = -1;
  fov->origin.y = -1;

  // make everything not visible
  SDL_memset(fov->visible.data,
             0,
             grid_count(&fov->visible) * sizeof(*fov->visible.data));

  // make everything unexplored
  SDL_memset(fov->explored.data,
             0,
             grid_count(&fov->explored) * sizeof(*fov->explored.data));
}

void
rl_update_fov(struct rl_fov* fov, grid(rl_tile) const* map, SDL_Point origin)
{
  SDL_assert(grid_same_shape(&fov->visible, map));
  SDL_assert(grid_same_shape(&fov->explored, map));

  if (fov->origin.x == origin.x && fov->origin.y == origin.y) {
    return;
  }

  fov->origin = origin;
  compute_fov(map, origin, fov->radius, &fov->visible);

  for (size_t i = 0; i < grid_count(&fov->visible); i++) {
    if (*grid_at_index(&fov->visible, i)) {
      *grid_at_index(&fov->explored, i) = true;
    }
  }
}
