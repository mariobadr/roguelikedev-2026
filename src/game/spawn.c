#include "spawn.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_log.h>

#include "container/array.h"
#include "container/grid.h"
#include "core/rand.h"

#include "actor.h"
#include "actor_def.h"
#include "generate.h"
#include "item_def.h"
#include "layout.h"
#include "level.h"
#include "loot.h"
#include "tile.h"
#include "world.h"

array_define_as(SDL_Point, rl_spawn_point);

/** When and how often an actor type spawns. */
struct rl_spawn_entry
{
  /** The actor to spawn. */
  enum rl_actor_type type;
  /** The shallowest depth it spawns at. */
  int min_depth;
  /** The deepest depth it spawns at, or 0 for no limit. */
  int max_depth;
  /** The relative chance of being picked. */
  int weight;
};

/** Which boss guards the stairs at a depth. */
struct rl_boss_entry
{
  /** The depth the boss spawns at. */
  int depth;
  /** The boss to spawn. */
  enum rl_actor_type type;
};

static struct rl_spawn_entry const ACTOR_SPAWNS[] = {
  { .type = RL_ACTOR_RAT, .min_depth = 1, .max_depth = 5, .weight = 15 },
  { .type = RL_ACTOR_GOBLIN, .min_depth = 2, .weight = 10 },
  { .type = RL_ACTOR_TROLL, .min_depth = 4, .weight = 5 },
};

static struct rl_boss_entry const BOSS_SPAWNS[] = {
  { .depth = 2, .type = RL_ACTOR_RAT_KING },
  { .depth = 5, .type = RL_ACTOR_GOBLIN_CHIEF },
  { .depth = 8, .type = RL_ACTOR_TROLL_WARLORD },
  { .depth = RL_FINAL_DEPTH, .type = RL_ACTOR_DRAGON },
};

static struct rl_loot_entry const FLOOR_ITEM_TYPES[] = {
  { .item = RL_ITEM_POTION_HEALTH, .weight = 2 },
  { .item = RL_ITEM_SCROLL_FIREBALL, .weight = 40 },
  { .item = RL_ITEM_SCROLL_LIGHTNING, .weight = 40 },
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

static bool
is_eligible(struct rl_spawn_entry const* entry, int depth)
{
  return depth >= entry->min_depth &&
         (entry->max_depth == 0 || depth <= entry->max_depth);
}

/**
 * @return an actor type appropriate for depth.
 */
static enum rl_actor_type
rl_gen_actor_type(int depth, struct rand_state* rng)
{
  int total = 0;
  for (size_t i = 0; i < SDL_arraysize(ACTOR_SPAWNS); i++) {
    if (is_eligible(&ACTOR_SPAWNS[i], depth)) {
      total += ACTOR_SPAWNS[i].weight;
    }
  }

  SDL_assert(total > 0);
  int roll = (int)rand_next_up_to(rng, total);
  for (size_t i = 0; i < SDL_arraysize(ACTOR_SPAWNS); i++) {
    if (!is_eligible(&ACTOR_SPAWNS[i], depth)) {
      continue;
    }
    roll -= ACTOR_SPAWNS[i].weight;
    if (roll < 0) {
      return ACTOR_SPAWNS[i].type;
    }
  }

  return ACTOR_SPAWNS[0].type;
}

/**
 * @return a level for an actor spawned at depth, no higher than depth.
 */
static int
rl_gen_actor_level(int depth, struct rand_state* rng)
{
  int const min = SDL_max(1, depth - 1);
  int const max = depth;

  return (int)rand_next_between(rng, min, max);
}

/**
 * @return a level for an item spawned at depth.
 */
static int
rl_gen_item_level(int depth, struct rand_state* rng)
{
  int const min = SDL_max(1, depth - 1);
  int const max = depth + 1;

  return (int)rand_next_between(rng, min, max);
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
            int actor_level,
            SDL_Point pos)
{
  handle(rl_actor) const actor_handle =
    rl_create_actor(world, type, actor_level);
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
    int const actor_level = rl_gen_actor_level(level->depth, rng);
    if (!spawn_actor(world, level, type, actor_level, *array_at(&points, i))) {
      ok = false;
      break;
    }
  }

  array_free(&points);
  return ok;
}

static struct rl_boss_entry const*
find_boss(int depth)
{
  for (size_t i = 0; i < SDL_arraysize(BOSS_SPAWNS); i++) {
    if (BOSS_SPAWNS[i].depth == depth) {
      return &BOSS_SPAWNS[i];
    }
  }

  return NULL;
}

bool
rl_spawn_boss(struct rl_level* level,
              struct rl_layout const* layout,
              struct rl_world* world,
              int boss_room)
{
  struct rl_boss_entry const* boss = find_boss(level->depth);
  if (boss == NULL) {
    return true;
  }

  SDL_Rect const* room = array_at(&layout->rooms, boss_room);
  SDL_Point best = { 0 };
  int best_dist = -1;
  for (int y = room->y; y < room->y + room->h; y++) {
    for (int x = room->x; x < room->x + room->w; x++) {
      SDL_Point const pos = { x, y };
      if (!can_spawn_at(level, world, pos)) {
        continue;
      }

      int const dx = x - level->stairs_down.x;
      int const dy = y - level->stairs_down.y;
      int const dist = dx * dx + dy * dy;
      if (best_dist < 0 || dist < best_dist) {
        best = pos;
        best_dist = dist;
      }
    }
  }

  if (best_dist < 0) {
    // no free tile; not a failure
    return true;
  }

  return spawn_actor(world, level, boss->type, level->depth, best);
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
    int const item_level = rl_gen_item_level(level->depth, rng);

    handle(rl_item) const item = rl_add_item_to_level(
      world, level, type, item_level, *array_at(&points, i));
    if (!handle_is_nonnull(item)) {
      ok = false;
      break;
    }
  }

  array_free(&points);
  return ok;
}
