#include "spawn.h"

#include "container/array.h"
#include "container/grid.h"
#include "core/rand.h"

#include "actor.h"
#include "actor_def.h"
#include "item_def.h"
#include "layout.h"
#include "level.h"
#include "tile.h"
#include "world.h"

array_define_as(SDL_Point, rl_spawn_point);

/**
 * @return how many actors should populate a level at depth.
 */
static int
rl_gen_total_actors(int depth, struct rand_state* rng)
{
  int const min = 7 + depth;
  int const max = 14 + depth * 2;

  return (int)rand_next_between(rng, min, max);
}

/**
 * @return an actor type appropriate for depth.
 */
static enum rl_actor_type
rl_gen_actor_type(int depth, struct rand_state* rng)
{
  (void)depth;
  (void)rng;

  // TODO: fix when there are more types
  return RL_ACTOR_RAT;
}

/**
 * @return how many items should populate a level.
 */
static int
rl_gen_total_items(struct rand_state* rng)
{
  return (int)rand_next_between(rng, 8, 20);
}

/**
 * @return an item type appropriate for depth.
 */
static enum rl_item_type
rl_gen_item_type(int depth, struct rand_state* rng)
{
  (void)depth;

  return (enum rl_item_type)rand_next_between(
    rng, RL_ITEM_POTION_HEALTH_MINOR, RL_ITEM_SCROLL_LIGHTNING);
}

static bool
can_spawn_at(struct rl_level const* level,
             struct rl_world const* world,
             SDL_Point pos)
{
  enum rl_tile const tile = *grid_at(&level->map, pos.x, pos.y);
  if (!rl_is_walkable(tile) || rl_is_staircase(tile)) {
    return false;
  }

  for (size_t i = 0; i < alist_len(&level->actors); i++) {
    struct rl_actor const* actor =
      rl_borrow_actor(world, *alist_at(&level->actors, i));
    if (actor == NULL) {
      continue;
    }

    if (actor->pos.x == pos.x && actor->pos.y == pos.y) {
      return false;
    }
  }

  for (size_t i = 0; i < alist_len(&level->items); i++) {
    struct rl_item const* item =
      rl_borrow_item(world, *alist_at(&level->items, i));
    if (item == NULL || item->ltype != RL_ITEM_LOCATION_MAP) {
      continue;
    }

    if (item->on.map.x == pos.x && item->on.map.y == pos.y) {
      return false;
    }
  }

  return true;
}

static void
find_spawn_points(array(rl_spawn_point) * out,
                  struct rl_level const* level,
                  struct rl_layout const* layout,
                  struct rl_world const* world,
                  int reserved_room,
                  struct rand_state* rng)
{
  array_clear(out);
  if (array_cap(out) == 0) {
    return;
  }

  size_t seen = 0;

  for (int i = 0; i < array_len(&layout->rooms); i++) {
    if (i == reserved_room) {
      continue;
    }

    SDL_Rect const* room = array_at(&layout->rooms, i);
    for (int y = room->y; y < room->y + room->h; y++) {
      for (int x = room->x; x < room->x + room->w; x++) {
        SDL_Point const pos = { x, y };
        if (!can_spawn_at(level, world, pos)) {
          continue;
        }

        seen++;
        if (!array_full(out)) {
          *array_push(out) = pos;
        } else {
          // Reservoir sampling keeps a uniform subset without storing all tiles.
          size_t const slot = (size_t)rand_next_up_to(rng, seen);
          if (slot < array_cap(out)) {
            *array_at(out, slot) = pos;
          }
        }
      }
    }
  }
}

static bool
spawn_actor(struct rl_world* world,
            struct rl_level* level,
            enum rl_actor_type type,
            SDL_Point pos)
{
  handle(rl_actor) const actor_handle = rl_create_actor(world, type);
  struct rl_actor* actor = rl_borrow_mut_actor(world, actor_handle);
  if (actor == NULL) {
    return false;
  }

  actor->pos = pos;

  if (!rl_add_actor(level, actor_handle)) {
    pool_release(&world->actors, actor_handle);
    return false;
  }

  return true;
}

bool
rl_spawn_actors(struct rl_level* level,
                struct rl_layout const* layout,
                struct rl_world* world,
                int reserved_room,
                struct rand_state* rng)
{
  int const total = rl_gen_total_actors(level->depth, rng);
  array(rl_spawn_point) points;
  if (!array_alloc(&points, total)) {
    return false;
  }

  find_spawn_points(&points, level, layout, world, reserved_room, rng);

  bool ok = true;
  for (size_t i = 0; i < array_len(&points); i++) {
    enum rl_actor_type const type = rl_gen_actor_type(level->depth, rng);
    if (!spawn_actor(world, level, type, *array_at(&points, i))) {
      ok = false;
      break;
    }
  }

  array_free(&points);
  return ok;
}

static bool
spawn_item(struct rl_world* world,
           struct rl_level* level,
           enum rl_item_type type,
           SDL_Point pos)
{
  handle(rl_item) const item_handle = rl_create_item(world, type);
  struct rl_item* item = rl_borrow_mut_item(world, item_handle);
  if (item == NULL) {
    return false;
  }

  item->ltype = RL_ITEM_LOCATION_MAP;
  item->on.map = pos;

  if (!rl_add_item(level, item_handle)) {
    pool_release(&world->items, item_handle);
    return false;
  }

  return true;
}

bool
rl_spawn_items(struct rl_level* level,
               struct rl_layout const* layout,
               struct rl_world* world,
               struct rand_state* rng)
{
  int const total = rl_gen_total_items(rng);
  array(rl_spawn_point) points;
  if (!array_alloc(&points, total)) {
    return false;
  }

  find_spawn_points(&points, level, layout, world, -1, rng);

  bool ok = true;
  for (size_t i = 0; i < array_len(&points); i++) {
    enum rl_item_type const type = rl_gen_item_type(level->depth, rng);
    if (!spawn_item(world, level, type, *array_at(&points, i))) {
      ok = false;
      break;
    }
  }

  array_free(&points);
  return ok;
}
