#include "layout.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "bsp.h"
#include "rand.h"

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
static inline int
axis_gap(struct extent a, struct extent b)
{
  if (b.start >= a.end) {
    return b.start - a.end;
  }

  if (a.start >= b.end) {
    return a.start - b.end;
  }

  return 0;
}

/**
 * @return the squared distance between the nearest edges of two rects.
 */
static int
rect_gap_dist_sq(SDL_Rect const* a, SDL_Rect const* b)
{
  struct extent a_x = { a->x, a->x + a->w };
  struct extent b_x = { b->x, b->x + b->w };
  struct extent a_y = { a->y, a->y + a->h };
  struct extent b_y = { b->y, b->y + b->h };

  int dx = axis_gap(a_x, b_x);
  int dy = axis_gap(a_y, b_y);

  return dx * dx + dy * dy;
}

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
    *out_a = a.end;     // a's wall tile facing b
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

  SDL_Log("Generated room: (%d, %d, %d, %d)", x, y, w, h);

  return rect;
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
make_corridor(SDL_Point a, SDL_Point b)
{
  struct rl_corridor corridor = { 0 };

  if (a.x == b.x) {
    corridor.segments[0] = make_vertical_line(a.y, b.y, b.x);
    corridor.segment_count = 1;
    SDL_Log("Generated vertical corridor: (%d, %d, %d)", a.y, b.y, b.x);
  } else if (a.y == b.y) {
    corridor.segments[0] = make_horizontal_line(a.x, b.x, a.y);
    corridor.segment_count = 1;
    SDL_Log("Generated horizontal corridor: (%d, %d, %d)", a.x, b.x, a.y);
  } else {
    // an L-shape
    corridor.segments[0] = make_horizontal_line(a.x, b.x, a.y);
    corridor.segments[1] = make_vertical_line(a.y, b.y, b.x);
    corridor.segment_count = 2;
    SDL_Log(
      "Generated L: (%d, %d, %d) + (%d, %d, %d)", a.x, b.x, a.y, a.y, b.y, b.x);
  }

  return corridor;
}

/**
 * A contiguous span of room indices.
 */
struct room_span
{
  int start;
  int count;
};

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
  struct rl_bsp_node const* node = &tree->nodes[node_index];

  if (rl_bsp_node_is_leaf(node)) {
    // base case
    int const room_index = out->room_count;

    out->rooms[room_index] = generate_room(&node->rect, rng);
    out->room_count++;

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

  int best_a = left.start, best_b = right.start;
  int best_dist =
    rect_center_dist_sq(&out->rooms[left.start], &out->rooms[right.start]);

  for (int i = left.start; i < left.start + left.count; i++) {
    for (int j = right.start; j < right.start + right.count; j++) {
      int d2 = rect_center_dist_sq(&out->rooms[i], &out->rooms[j]);
      if (d2 < best_dist) {
        best_dist = d2;
        best_a = i;
        best_b = j;
      }
    }
  }

  // connect best_a and best_b with a corridor
  SDL_Point door_a, door_b;
  nearest_points(&out->rooms[best_a], &out->rooms[best_b], &door_a, &door_b);
  out->corridors[out->corridor_count] = make_corridor(door_a, door_b);
  out->corridor_count++;

  struct room_span r = { 0 };
  r.start = left.start;
  r.count = left.count + right.count;

  return r;
}

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

  layout->rooms = SDL_calloc(tree.leaf_count, sizeof(*layout->rooms));
  if (layout->rooms == NULL) {
    SDL_Log("SDL_calloc failed: %s", SDL_GetError());
    rl_free_layout(layout);

    rl_bsp_tree_free(&tree);
    return false;
  }

  layout->corridors =
    SDL_calloc(tree.leaf_count - 1, sizeof(*layout->corridors));
  if (layout->corridors == NULL) {
    SDL_Log("SDL_calloc failed: %s", SDL_GetError());
    rl_free_layout(layout);

    rl_bsp_tree_free(&tree);
    return false;
  }

  connect_bsp_subtree(layout, &tree, 0, rng);

  rl_bsp_tree_free(&tree);
  return true;
}

void
rl_free_layout(struct rl_layout* layout)
{
  if (layout == NULL) {
    return;
  }

  SDL_free(layout->rooms);
  layout->rooms = NULL;
  layout->room_count = 0;

  SDL_free(layout->corridors);
  layout->corridors = NULL;
  layout->corridor_count = 0;
}
