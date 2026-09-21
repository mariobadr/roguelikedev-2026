#include "spawn.h"

#include <SDL3/SDL_log.h>

#include "container/array.h"
#include "container/grid.h"
#include "core/rand.h"

#include "actor.h"
#include "actor_def.h"
#include "item_def.h"
#include "layout.h"
#include "level.h"
#include "loot.h"
#include "tile.h"
#include "world.h"

array_define_as(SDL_Point, rl_spawn_point);

static struct rl_loot_entry const FLOOR_ITEM_TYPES[] = {
  { .item = RL_ITEM_POTION_HEALTH_MINOR, .weight = 50 },
  { .item = RL_ITEM_SCROLL_FIREBALL, .weight = 25 },
  { .item = RL_ITEM_SCROLL_LIGHTNING, .weight = 25 },
};

static struct rl_loot_table const FLOOR_ITEMS = {
  .drop_percent = 100,
  .items = FLOOR_ITEM_TYPES,
  .count = SDL_arraysize(FLOOR_ITEM_TYPES),
};

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
  return (int)rand_next_between(rng, 2, 5);
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
    if (actor->pos.x == pos.x && actor->pos.y == pos.y) {
      return false;
    }
  }

  for (size_t i = 0; i < alist_len(&level->items); i++) {
    struct rl_item const* item =
      rl_borrow_item(world, *alist_at(&level->items, i));
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
          // Reservoir sampling keeps a uniform subset without storing all
          // tiles.
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
  handle(rl_actor) const actor_handle =
    rl_create_actor(world, type, level->depth);
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
    enum rl_item_type type;
    rl_roll_loot(&FLOOR_ITEMS, rng, &type);

    handle(rl_item) const item =
      rl_add_item_to_level(world, level, type, *array_at(&points, i));
    if (!handle_is_nonnull(item)) {
      ok = false;
      break;
    }
  }

  array_free(&points);
  return ok;
}
