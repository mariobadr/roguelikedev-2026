#include "spawn.h"

#include "container/array.h"
#include "container/grid.h"

#include "procgen/rand.h"

#include "actor.h"
#include "level.h"
#include "tile.h"

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
  return (int)rand_next_between(rng, 5, 12);
}

/**
 * @return an item type appropriate for depth.
 */
static enum rl_item_type
rl_gen_item_type(int depth, struct rand_state* rng)
{
  (void)depth;
  (void)rng;

  // TODO: fix when there are more types
  return RL_ITEM_POTION_HEALTH_MINOR;
}

static bool
is_tile_free(grid(rl_tile) const* map,
             alist(rl_actor) const* actors,
             SDL_Point pos)
{
  if (!rl_is_walkable(*grid_at(map, pos.x, pos.y))) {
    return false;
  }

  for (size_t i = 0; i < alist_len(actors); i++) {
    struct rl_actor const* actor = alist_at(actors, i);
    if (actor->pos.x == pos.x && actor->pos.y == pos.y) {
      return false;
    }
  }

  return true;
}

static bool
pick_free_tile(SDL_Point* out,
               SDL_Rect const* room,
               grid(rl_tile) const* map,
               alist(rl_actor) const* actors,
               struct rand_state* rng)
{
  int seen = 0;

  for (int y = room->y; y < room->y + room->h; y++) {
    for (int x = room->x; x < room->x + room->w; x++) {
      SDL_Point const pos = { x, y };
      if (!is_tile_free(map, actors, pos)) {
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
                 alist(rl_actor) const* actors,
                 struct rand_state* rng)
{
  while (!array_empty(eligible)) {
    int const slot = (int)rand_next_up_to(rng, array_len(eligible));
    int const room_index = *array_at(eligible, slot);
    SDL_Rect const* room = array_at(&level->layout.rooms, room_index);

    if (pick_free_tile(out, room, &level->map, actors, rng)) {
      return true;
    }

    int const last = (int)array_len(eligible) - 1;
    *array_at(eligible, slot) = *array_at(eligible, last);
    array_pop(eligible);
  }

  return false;
}

static bool
add_actor(alist(rl_actor) * actors, enum rl_actor_type type, SDL_Point pos)
{
  struct rl_actor* new_actor = alist_push(actors);
  if (new_actor == NULL) {
    return false;
  }

  *new_actor = rl_create_actor(type, (int)alist_len(actors));
  new_actor->pos = pos;

  return true;
}

bool
rl_spawn_actors(struct rl_level const* level,
                alist(rl_actor) * actors,
                int reserved_room,
                struct rand_state* rng)
{
  int const room_count = (int)array_len(&level->layout.rooms);

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
    if (!find_spawn_point(&pos, &eligible, level, actors, rng)) {
      ok = false;
      break;
    }

    enum rl_actor_type const type = rl_gen_actor_type(level->depth, rng);
    if (!add_actor(actors, type, pos)) {
      ok = false;
      break;
    }
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
               alist(rl_item) * items,
               struct rand_state* rng)
{
  int const room_count = (int)array_len(&level->layout.rooms);
  int const total = rl_gen_total_items(rng);

  bool ok = true;
  for (int i = 0; i < total; i++) {
    int const room_index = (int)rand_next_up_to(rng, room_count);
    SDL_Rect const* room = array_at(&level->layout.rooms, room_index);

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
