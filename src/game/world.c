#include "world.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "spawn.h"

bool
rl_alloc_world(struct rl_world* world, int width, int height)
{
  // TODO: need an alist of levels
  if (!rl_alloc_level(&world->level, 1, width, height)) {
    rl_free_world(world);
    return false;
  }

  // allocate space for the actors
  if (!alist_alloc(&world->actors, 16)) {
    SDL_Log("alist_alloc failed: %s", SDL_GetError());
    rl_free_world(world);
    return false;
  }

  // allocate space for the items
  if (!alist_alloc(&world->items, 8)) {
    SDL_Log("alist_alloc failed: %s", SDL_GetError());
    rl_free_world(world);
    return false;
  }

  // the main character
  world->rogue = rl_create_actor(RL_ACTOR_ROGUE, RL_ROGUE_ID);

  return true;
}

void
rl_free_world(struct rl_world* world)
{
  if (world == NULL) {
    return;
  }

  alist_free(&world->items);
  alist_free(&world->actors);
  rl_free_level(&world->level);
}

struct rl_actor const*
rl_get_actor(struct rl_world const* world, int id)
{
  if (id == RL_ROGUE_ID) {
    return &world->rogue;
  }

  int const index = id - 1;
  if (index < 0 || index >= alist_len(&world->actors)) {
    return NULL;
  }

  return alist_at(&world->actors, index);
}

struct rl_actor*
rl_edit_actor(struct rl_world* world, int id)
{
  if (id == RL_ROGUE_ID) {
    return &world->rogue;
  }

  int const index = id - 1;
  if (index < 0 || index >= alist_len(&world->actors)) {
    return NULL;
  }

  return alist_at(&world->actors, index);
}

int
rl_actor_count(struct rl_world const* world)
{
  return 1 + (int)alist_len(&world->actors);
}

struct rl_item const*
rl_get_item(struct rl_world const* world, int id)
{
  if (id < 0 || id >= alist_len(&world->items)) {
    return NULL;
  }

  return alist_at(&world->items, id);
}

struct rl_actor const*
rl_find_actor(struct rl_world const* world, SDL_Point pos)
{
  for (int id = 0; id < rl_actor_count(world); id++) {
    struct rl_actor const* actor = rl_get_actor(world, id);
    if (!rl_actor_is_alive(actor)) {
      // ignore dead actors
      continue;
    }

    if (actor->pos.x == pos.x && actor->pos.y == pos.y) {
      return actor;
    }
  }

  return NULL;
}

struct rl_item*
rl_find_item(struct rl_world* world, SDL_Point pos)
{
  for (int id = 0; id < alist_len(&world->items); id++) {
    struct rl_item* item = alist_at(&world->items, id);
    if (item->ltype != RL_ITEM_LOCATION_MAP) {
      // ignore items not on the map
      continue;
    }

    if (item->on.map.x == pos.x && item->on.map.y == pos.y) {
      return item;
    }
  }

  return NULL;
}

struct rl_level const*
rl_get_current_level(struct rl_world const* world)
{
  return &world->level;
}

struct rl_level*
rl_edit_current_level(struct rl_world* world)
{
  return &world->level;
}
