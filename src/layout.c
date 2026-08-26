#include "layout.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "bsp.h"
#include "rand.h"

/* Room generation */

/**
 * @return a randomly generated rectangle within bounds.
 */
static SDL_Rect
generate_room(SDL_Rect const* bounds, struct rand_state* rng)
{
  int const margin = 1;
  // how much smaller than the leaf a room can be, per axis
  int const min_shave = 1;
  int const max_shave = 3;

  int shave_w = (int)rand_next_between(rng, min_shave, max_shave);
  int shave_h = (int)rand_next_between(rng, min_shave, max_shave);

  int w = bounds->w - 2 * margin - shave_w;
  int h = bounds->h - 2 * margin - shave_h;

  // room can sit anywhere within the space freed up by the shave
  int x = bounds->x + margin + (int)rand_next_between(rng, 0, shave_w);
  int y = bounds->y + margin + (int)rand_next_between(rng, 0, shave_h);

  SDL_Rect rect = { 0 };
  rect.x = x;
  rect.y = y;
  rect.w = w;
  rect.h = h;

  return rect;
}

/* Corridor generation */

/**
 * The interval [start, end).
 */
struct extent
{
  int start;
  int end;
};

/**
 * @return the gap between two extents on one axis, or 0 if they overlap.
 */
// static inline int
// axis_gap(struct extent a, struct extent b)
// {
//   if (b.start >= a.end) {
//     return b.start - a.end;
//   }

//   if (a.start >= b.end) {
//     return a.start - b.end;
//   }

//   return 0;
// }

/**
 * @return the squared distance between the nearest edges of two rects.
 */
// static int
// rect_gap_dist_sq(SDL_Rect const* a, SDL_Rect const* b)
// {
//   struct extent a_x = { a->x, a->x + a->w };
//   struct extent b_x = { b->x, b->x + b->w };
//   struct extent a_y = { a->y, a->y + a->h };
//   struct extent b_y = { b->y, b->y + b->h };

//   int dx = axis_gap(a_x, b_x);
//   int dy = axis_gap(a_y, b_y);

//   return dx * dx + dy * dy;
// }

/**
 * @return the squared distance between the centers of two rects.
 */
static int
rect_center_dist_sq(SDL_Rect const* a, SDL_Rect const* b)
{
  int ax = a->x + a->w / 2, ay = a->y + a->h / 2;
  int bx = b->x + b->w / 2, by = b->y + b->h / 2;
  int dx = ax - bx, dy = ay - by;
  return dx * dx + dy * dy;
}

/**
 * Finds the nearest facing coordinates of two extents on one axis.
 */
static void
nearest_on_axis(struct extent a, struct extent b, int* out_a, int* out_b)
{
  if (b.start >= a.end) {
    *out_a = a.end;       // a's wall tile facing b
    *out_b = b.start - 1; // b's wall tile facing a
  } else if (a.start >= b.end) {
    *out_a = a.start - 1;
    *out_b = b.end;
  } else {
    // ranges overlap on this axis -- any shared coordinate works
    int lo = (a.start > b.start) ? a.start : b.start;
    int hi = (a.end < b.end) ? a.end : b.end;
    *out_a = *out_b = (lo + hi - 1) / 2;
  }
}

/**
 * Finds the nearest facing points on two rects a and b.
 */
static void
nearest_points(SDL_Rect const* a,
               SDL_Rect const* b,
               SDL_Point* pa,
               SDL_Point* pb)
{
  struct extent a_x = { a->x, a->x + a->w };
  struct extent b_x = { b->x, b->x + b->w };
  struct extent a_y = { a->y, a->y + a->h };
  struct extent b_y = { b->y, b->y + b->h };

  nearest_on_axis(a_x, b_x, &pa->x, &pb->x);
  nearest_on_axis(a_y, b_y, &pa->y, &pb->y);
}

/**
 * @return a 1-tile-tall rect spanning from x1 to x2 at row y.
 */
static SDL_Rect
make_horizontal_line(int x1, int x2, int y)
{
  // ensure x2 > x1 by swapping their values
  if (x1 > x2) {
    int tmp = x1;
    x1 = x2;
    x2 = tmp;
  }

  SDL_Rect segment = { 0 };
  segment.x = x1;
  segment.y = y;
  segment.w = x2 - x1 + 1;
  segment.h = 1;

  return segment;
}

/**
 * @return a 1-tile-wide rect spanning from y1 to y2 at column x.
 */
static SDL_Rect
make_vertical_line(int y1, int y2, int x)
{
  // ensure y2 > y1 by swapping their values
  if (y1 > y2) {
    int tmp = y1;
    y1 = y2;
    y2 = tmp;
  }

  SDL_Rect segment = { 0 };
  segment.x = x;
  segment.y = y1;
  segment.w = 1;
  segment.h = y2 - y1 + 1;

  return segment;
}

/**
 * @return a (straight or L-shaped) corridor connecting two points.
 */
static struct rl_corridor
make_corridor(SDL_Point a, SDL_Point b, int room_a, int room_b)
{
  struct rl_corridor corridor = { 0 };
  corridor.room_a = room_a;
  corridor.room_b = room_b;

  if (a.x == b.x) {
    corridor.segments[0] = make_vertical_line(a.y, b.y, b.x);
    corridor.segment_count = 1;
  } else if (a.y == b.y) {
    corridor.segments[0] = make_horizontal_line(a.x, b.x, a.y);
    corridor.segment_count = 1;
  } else {
    // an L-shape
    corridor.segments[0] = make_horizontal_line(a.x, b.x, a.y);
    corridor.segments[1] = make_vertical_line(a.y, b.y, b.x);
    corridor.segment_count = 2;
  }

  return corridor;
}

/* Extra corridor generation */

enum quadrant
{
  QUADRANT_TL, //< top-left
  QUADRANT_TR, //< top-right
  QUADRANT_BL, //< bottom-left
  QUADRANT_BR, //< bottom-right
};

/**
 * @return which map quadrant a room's center falls into.
 */
static enum quadrant
get_quadrant(SDL_Rect const* room, int width, int height)
{
  int cx = room->x + room->w / 2, cy = room->y + room->h / 2;
  bool right = cx >= width / 2;
  bool bottom = cy >= height / 2;

  if (!bottom && !right) {
    return QUADRANT_TL;
  }

  if (!bottom && right) {
    return QUADRANT_TR;
  }

  if (bottom && !right) {
    return QUADRANT_BL;
  }

  return QUADRANT_BR;
}

/**
 * @return true if a corridor already connects the given two rooms, in
 * either direction.
 */
static bool
corridor_exists(struct rl_layout const* out, int room_a, int room_b)
{
  for (int i = 0; i < array_len(&out->corridors); i++) {
    struct rl_corridor const* c = array_at(&out->corridors, i);
    if ((c->room_a == room_a && c->room_b == room_b) ||
        (c->room_a == room_b && c->room_b == room_a)) {
      return true;
    }
  }
  return false;
}

/**
 * Finds the closest pair of rooms (by center distance) with one room in
 * quadrant qa and the other in quadrant qb.
 *
 * @return false if either quadrant has no rooms in it.
 */
static bool
find_closest_pair_in_quadrants(array(rl_room) const* rooms,
                               int width,
                               int height,
                               enum quadrant qa,
                               enum quadrant qb,
                               int* out_a,
                               int* out_b)
{
  bool found = false;
  int best_a = 0, best_b = 0, best_dist = 0;

  for (int i = 0; i < array_len(rooms); i++) {
    if (get_quadrant(array_at(rooms, i), width, height) != qa) {
      // rooms[i] is not in quadrant a
      continue;
    }

    for (int j = 0; j < array_len(rooms); j++) {
      if (get_quadrant(array_at(rooms, j), width, height) != qb) {
        // rooms[j] is not in quadrant b
        continue;
      }

      int d2 = rect_center_dist_sq(array_at(rooms, i), array_at(rooms, j));
      if (!found || d2 < best_dist) {
        found = true;
        best_dist = d2;
        best_a = i;
        best_b = j;
      }
    }
  }

  if (found) {
    *out_a = best_a;
    *out_b = best_b;
  }

  return found;
}

/**
 * Connect the closest pair of rooms across two quadrants with a corridor.
 */
static bool
connect_quadrants(struct rl_layout* out,
                  int width,
                  int height,
                  enum quadrant qa,
                  enum quadrant qb)
{
  int best_a, best_b;
  if (!find_closest_pair_in_quadrants(
        &out->rooms, width, height, qa, qb, &best_a, &best_b)) {
    // one of the quadrants had no rooms in it - possibe?
    return false;
  }

  if (corridor_exists(out, best_a, best_b)) {
    return false;
  }

  SDL_Point door_a, door_b;
  nearest_points(array_at(&out->rooms, best_a),
                 array_at(&out->rooms, best_b),
                 &door_a,
                 &door_b);
  *array_push(&out->corridors) = make_corridor(door_a, door_b, best_a, best_b);

  return true;
}

/**
 * Add up to 4 extra corridors connecting the map's quadrants.
 */
static void
add_extra_corridors(struct rl_layout* out, int width, int height)
{
  connect_quadrants(out, width, height, QUADRANT_TL, QUADRANT_TR);
  connect_quadrants(out, width, height, QUADRANT_TL, QUADRANT_BL);
  connect_quadrants(out, width, height, QUADRANT_TR, QUADRANT_BR);
  connect_quadrants(out, width, height, QUADRANT_BL, QUADRANT_BR);
}

/* Layout generation */

/**
 * A contiguous span of room indices.
 */
struct room_span
{
  int start;
  int count;
};

/**
 * Finds the closest pair of rooms between two room spans.
 */
static void
find_closest_pair_in_spans(array(rl_room) const* rooms,
                           struct room_span a,
                           struct room_span b,
                           int* out_a,
                           int* out_b)
{
  int best_a = a.start, best_b = b.start;
  int best_dist =
    rect_center_dist_sq(array_at(rooms, a.start), array_at(rooms, b.start));

  for (int i = a.start; i < a.start + a.count; i++) {
    for (int j = b.start; j < b.start + b.count; j++) {
      int d2 = rect_center_dist_sq(array_at(rooms, i), array_at(rooms, j));
      if (d2 < best_dist) {
        best_dist = d2;
        best_a = i;
        best_b = j;
      }
    }
  }

  *out_a = best_a;
  *out_b = best_b;
}

/**
 * Recursively builds rooms from a BSP subtree and connects them with corridors.
 *
 * @return the span of room indices generated for this subtree.
 */
static struct room_span
connect_bsp_subtree(struct rl_layout* out,
                    struct rl_bsp_tree const* tree,
                    int node_index,
                    struct rand_state* rng)
{
  struct rl_bsp_node const* node = array_at(&tree->nodes, node_index);

  if (rl_bsp_node_is_leaf(node)) {
    // base case
    int const room_index = (int)array_len(&out->rooms);
    *array_push(&out->rooms) = generate_room(&node->rect, rng);

    // this is a leaf, so only one room here
    struct room_span r = { 0 };
    r.start = room_index;
    r.count = 1;

    return r;
  }

  // recurse left and right subtrees
  struct room_span left =
    connect_bsp_subtree(out, tree, rl_bsp_left_of(node_index), rng);
  struct room_span right =
    connect_bsp_subtree(out, tree, rl_bsp_right_of(node_index), rng);

  int best_a, best_b;
  find_closest_pair_in_spans(&out->rooms, left, right, &best_a, &best_b);

  // connect best_a and best_b with a corridor
  SDL_Point door_a, door_b;
  nearest_points(array_at(&out->rooms, best_a),
                 array_at(&out->rooms, best_b),
                 &door_a,
                 &door_b);
  *array_push(&out->corridors) = make_corridor(door_a, door_b, best_a, best_b);

  struct room_span r = { 0 };
  r.start = left.start;
  r.count = left.count + right.count;

  return r;
}

/* Public API */

bool
rl_init_layout(struct rl_layout* layout,
               int width,
               int height,
               struct rand_state* rng)
{
  SDL_Rect rect = { 0 };
  rect.w = width;
  rect.h = height;

  int const max_depth = 4;
  struct rl_bsp_tree tree;
  if (!rl_bsp_tree_init(&tree, max_depth, rect)) {
    return false;
  }

  struct rl_bsp_policy policy;
  policy.min_width = 10;
  policy.min_height = 8;
  policy.max_wh_ratio = 2.0;
  policy.max_hw_ratio = 2.0;
  rl_bsp_split(&tree, 0, rng, 0, &policy);

  if(!array_alloc(&layout->rooms, tree.leaf_count)) {
    SDL_Log("array_alloc failed: %s", SDL_GetError());
    rl_bsp_tree_free(&tree);
    return false;
  }

  if (!array_alloc(&layout->corridors, tree.leaf_count - 1 + 4)) {
    SDL_Log("array_alloc failed: %s", SDL_GetError());
    rl_bsp_tree_free(&tree);
    rl_free_layout(layout);
    return false;
  }

  connect_bsp_subtree(layout, &tree, 0, rng);
  add_extra_corridors(layout, width, height);

  rl_bsp_tree_free(&tree);
  return true;
}

void
rl_free_layout(struct rl_layout* layout)
{
  if (layout == NULL) {
    return;
  }

  array_free(&layout->rooms);
  array_free(&layout->corridors);
}
