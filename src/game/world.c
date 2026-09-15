#include "world.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

bool
rl_alloc_world(struct rl_world* world)
{
  world->rogue = handle_invalid(rl_actor);

  // allocate space for the levels
  if (!alist_alloc(&world->levels, 4)) {
    SDL_Log("alist_alloc failed: %s", SDL_GetError());
    return false;
  }

  // allocate space for the actors
  if (!alist_alloc(&world->actors, 16)) {
    SDL_Log("alist_alloc failed: %s", SDL_GetError());
    return false;
  }

  // allocate space for the items
  if (!alist_alloc(&world->items, 8)) {
    SDL_Log("alist_alloc failed: %s", SDL_GetError());
    return false;
  }

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

  for (size_t i = 0; i < alist_len(&world->levels); i++) {
    rl_free_level(alist_at(&world->levels, i));
  }
  alist_free(&world->levels);
}

struct rl_actor*
rl_add_actor(struct rl_world* world, enum rl_actor_type type)
{
  struct rl_actor* actor = alist_push(&world->actors);
  if (actor == NULL) {
    return NULL;
  }

  Uint32 const index = (Uint32)(alist_len(&world->actors) - 1);
  *actor = rl_create_actor(type);
  actor->handle = (handle(rl_actor)){ index, 1 };

  return actor;
}

handle(rl_actor) rl_rogue_handle(struct rl_world const* world)
{
  return world->rogue;
}

struct rl_actor const*
rl_get_actor(struct rl_world const* world, handle(rl_actor) actor_handle)
{
  if (actor_handle.index >= alist_len(&world->actors)) {
    return NULL;
  }

  struct rl_actor const* actor = alist_at(&world->actors, actor_handle.index);
  return handle_equal(actor->handle, actor_handle) ? actor : NULL;
}

struct rl_actor*
rl_edit_actor(struct rl_world* world, handle(rl_actor) actor_handle)
{
  if (actor_handle.index >= alist_len(&world->actors)) {
    return NULL;
  }

  struct rl_actor* actor = alist_at(&world->actors, actor_handle.index);
  return handle_equal(actor->handle, actor_handle) ? actor : NULL;
}

int
rl_actor_count(struct rl_world const* world)
{
  return (int)alist_len(&world->actors);
}

struct rl_actor const*
rl_get_actor_at(struct rl_world const* world, int index)
{
  if (index < 0 || index >= alist_len(&world->actors)) {
    return NULL;
  }

  return alist_at(&world->actors, index);
}

struct rl_actor*
rl_edit_actor_at(struct rl_world* world, int index)
{
  if (index < 0 || index >= alist_len(&world->actors)) {
    return NULL;
  }

  return alist_at(&world->actors, index);
}

struct rl_item const*
rl_get_item(struct rl_world const* world, int id)
{
  if (id < 0 || id >= alist_len(&world->items)) {
    return NULL;
  }

  return alist_at(&world->items, id);
}

struct rl_item*
rl_edit_item(struct rl_world* world, int id)
{
  if (id < 0 || id >= alist_len(&world->items)) {
    return NULL;
  }

  return alist_at(&world->items, id);
}

struct rl_actor const*
rl_find_actor(struct rl_world const* world, SDL_Point pos)
{
  for (int i = 0; i < rl_actor_count(world); i++) {
    struct rl_actor const* actor = rl_get_actor_at(world, i);
    if (actor == NULL || !rl_actor_is_alive(actor)) {
      // ignore empty slots and dead actors
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
  struct rl_actor const* rogue = rl_get_actor(world, rl_rogue_handle(world));
  if (rogue == NULL) {
    return NULL;
  }

  int const index = rogue->level;
  if (index < 0 || index >= alist_len(&world->levels)) {
    return NULL;
  }

  return alist_at(&world->levels, index);
}

struct rl_level*
rl_edit_current_level(struct rl_world* world)
{
  struct rl_actor const* rogue = rl_get_actor(world, rl_rogue_handle(world));
  if (rogue == NULL) {
    return NULL;
  }

  int const index = rogue->level;
  if (index < 0 || index >= alist_len(&world->levels)) {
    return NULL;
  }

  return alist_at(&world->levels, index);
}
