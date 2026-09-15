#include "world.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

bool
rl_alloc_world(struct rl_world* world)
{
  world->rogue = handle_invalid(rl_actor);
  world->current_level = -1;

  // allocate space for the levels
  if (!alist_alloc(&world->levels, 4)) {
    SDL_Log("alist_alloc failed: %s", SDL_GetError());
    return false;
  }

  // allocate space for the actors
  if (!pool_alloc(&world->actors, 16)) {
    SDL_Log("pool_alloc failed: %s", SDL_GetError());
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
  pool_free(&world->actors);

  for (size_t i = 0; i < alist_len(&world->levels); i++) {
    rl_free_level(alist_at(&world->levels, i));
  }
  alist_free(&world->levels);
}

handle(rl_actor)
rl_create_actor(struct rl_world* world, enum rl_actor_type type)
{
  if (pool_full(&world->actors) &&
      !pool_reserve(&world->actors, (size_t)pool_cap(&world->actors) * 2)) {
    return handle_invalid(rl_actor);
  }

  handle(rl_actor) actor_handle;
  struct rl_actor* actor = pool_acquire(&world->actors, &actor_handle);
  if (actor == NULL) {
    return handle_invalid(rl_actor);
  }

  *actor = rl_make_actor(type);
  actor->handle = actor_handle;

  return actor_handle;
}

handle(rl_actor) rl_get_rogue(struct rl_world const* world)
{
  return world->rogue;
}

struct rl_actor const*
rl_borrow_actor(struct rl_world const* world, handle(rl_actor) actor_handle)
{
  return pool_get(&world->actors, actor_handle);
}

struct rl_actor*
rl_borrow_mut_actor(struct rl_world* world, handle(rl_actor) actor_handle)
{
  return pool_get(&world->actors, actor_handle);
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

handle(rl_actor)
rl_find_actor(struct rl_world const* world,
              struct rl_level const* level,
              SDL_Point pos)
{
  if (level == NULL) {
    return handle_invalid(rl_actor);
  }

  for (size_t i = 0; i < alist_len(&level->actors); i++) {
    handle(rl_actor) const actor_handle = *alist_at(&level->actors, i);
    struct rl_actor const* actor = rl_borrow_actor(world, actor_handle);
    if (actor == NULL || !rl_actor_is_alive(actor)) {
      // ignore stale handles and dead actors
      continue;
    }

    if (actor->pos.x == pos.x && actor->pos.y == pos.y) {
      return actor_handle;
    }
  }

  return handle_invalid(rl_actor);
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
  int const index = world->current_level;
  if (index < 0 || index >= alist_len(&world->levels)) {
    return NULL;
  }

  return alist_at(&world->levels, index);
}

struct rl_level*
rl_edit_current_level(struct rl_world* world)
{
  int const index = world->current_level;
  if (index < 0 || index >= alist_len(&world->levels)) {
    return NULL;
  }

  return alist_at(&world->levels, index);
}
