#include "spawn.h"

#include "container/array.h"
#include "container/grid.h"

#include "procgen/layout.h"
#include "procgen/rand.h"

#include "actor.h"
#include "level.h"
#include "tile.h"
#include "world.h"

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
is_tile_free(struct rl_level const* level,
             struct rl_world const* world,
             SDL_Point pos)
{
  if (!rl_is_walkable(*grid_at(&level->map, pos.x, pos.y))) {
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

  return true;
}

static bool
pick_free_tile(SDL_Point* out,
               SDL_Rect const* room,
               struct rl_level const* level,
               struct rl_world const* world,
               struct rand_state* rng)
{
  int seen = 0;

  for (int y = room->y; y < room->y + room->h; y++) {
    for (int x = room->x; x < room->x + room->w; x++) {
      SDL_Point const pos = { x, y };
      if (!is_tile_free(level, world, pos)) {
        continue;
      }

      seen++;
      if (rand_next_up_to(rng, seen) == 0) {
        *out = pos;
      }
    }
  }

  return seen > 0;
}

static bool
find_spawn_point(SDL_Point* out,
                 array(int) * eligible,
                 struct rl_level const* level,
                 struct rl_layout const* layout,
                 struct rl_world const* world,
                 struct rand_state* rng)
{
  while (!array_empty(eligible)) {
    int const slot = (int)rand_next_up_to(rng, array_len(eligible));
    int const room_index = *array_at(eligible, slot);
    SDL_Rect const* room = array_at(&layout->rooms, room_index);

    if (pick_free_tile(out, room, level, world, rng)) {
      return true;
    }

    int const last = (int)array_len(eligible) - 1;
    *array_at(eligible, slot) = *array_at(eligible, last);
    array_pop(eligible);
  }

  return false;
}

static struct rl_actor*
spawn_actor(struct rl_world* world,
            struct rl_level* level,
            enum rl_actor_type type)
{
  handle(rl_actor) h = rl_create_actor(world, type);
  struct rl_actor* actor = rl_borrow_mut_actor(world, h);
  if (actor == NULL) {
    return NULL;
  }

  if (!rl_add_actor(level, actor->handle)) {
    pool_release(&world->actors, actor->handle);
    return NULL;
  }

  return actor;
}

bool
rl_spawn_actors(struct rl_level* level,
                struct rl_layout const* layout,
                struct rl_world* world,
                int reserved_room,
                struct rand_state* rng)
{
  int const room_count = (int)array_len(&layout->rooms);

  array(int) eligible;
  if (!array_alloc(&eligible, room_count)) {
    return false;
  }

  for (int i = 0; i < room_count; i++) {
    if (i != reserved_room) {
      *array_push(&eligible) = i;
    }
  }

  bool ok = true;
  int const total = rl_gen_total_actors(level->depth, rng);
  for (int i = 0; i < total && !array_empty(&eligible); i++) {
    SDL_Point pos = { 0 };
    if (!find_spawn_point(&pos, &eligible, level, layout, world, rng)) {
      ok = false;
      break;
    }

    enum rl_actor_type const type = rl_gen_actor_type(level->depth, rng);
    struct rl_actor* actor = spawn_actor(world, level, type);
    if (actor == NULL) {
      ok = false;
      break;
    }

    actor->pos = pos;
  }

  array_free(&eligible);
  return ok;
}

static bool
add_item(alist(rl_item) * items, enum rl_item_type type, SDL_Point pos)
{
  struct rl_item* new_item = alist_push(items);
  if (new_item == NULL) {
    return false;
  }

  new_item->itype = type;
  new_item->ltype = RL_ITEM_LOCATION_MAP;
  new_item->on.map = pos;
  new_item->id = (int)alist_len(items) - 1;

  return true;
}

bool
rl_spawn_items(struct rl_level const* level,
               struct rl_layout const* layout,
               alist(rl_item) * items,
               struct rand_state* rng)
{
  int const room_count = (int)array_len(&layout->rooms);
  int const total = rl_gen_total_items(rng);

  bool ok = true;
  for (int i = 0; i < total; i++) {
    int const room_index = (int)rand_next_up_to(rng, room_count);
    SDL_Rect const* room = array_at(&layout->rooms, room_index);

    SDL_Point pos;
    pos.x = (int)rand_next_between(rng, room->x, room->x + room->w - 1);
    pos.y = (int)rand_next_between(rng, room->y, room->y + room->h - 1);

    enum rl_item_type const type = rl_gen_item_type(level->depth, rng);
    if (!add_item(items, type, pos)) {
      ok = false;
      break;
    }
  }

  return ok;
}
