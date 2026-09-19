#include "world.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

bool
rl_alloc_world(struct rl_world* world)
{
  world->player.actor = handle_invalid(rl_actor);
  world->player.xp = 0;
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
  if (!pool_alloc(&world->items, 8)) {
    SDL_Log("pool_alloc failed: %s", SDL_GetError());
    return false;
  }

  return true;
}

static void
update_explored(struct rl_level* level, struct rl_fov const* fov)
{
  for (size_t i = 0; i < grid_count(&fov->visible); i++) {
    if (*grid_at_index(&fov->visible, i)) {
      *grid_at_index(&level->explored, i) = true;
    }
  }
}

void
rl_update_visibility(struct rl_world* world)
{
  struct rl_level* level = rl_edit_current_level(world);
  struct rl_actor const* rogue =
    rl_borrow_actor(world, rl_get_rogue(world));

  rl_update_fov(&world->player.fov, &level->map, rogue->pos);
  update_explored(level, &world->player.fov);
}

bool
rl_create_player(struct rl_world* world)
{
  struct rl_level const* level = rl_get_current_level(world);

  if (!rl_alloc_player(
        &world->player, grid_width(&level->map), grid_height(&level->map))) {
    return false;
  }

  rl_update_visibility(world);
  return true;
}

void
rl_free_world(struct rl_world* world)
{
  if (world == NULL) {
    return;
  }

  rl_free_player(&world->player);
  pool_free(&world->items);
  pool_free(&world->actors);

  for (size_t i = 0; i < alist_len(&world->levels); i++) {
    rl_free_level(alist_at(&world->levels, i));
  }
  alist_free(&world->levels);
}

handle(rl_actor)
rl_create_actor(struct rl_world* world, enum rl_actor_type type, int level)
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

  *actor = rl_make_actor(type, level);
  actor->handle = actor_handle;

  return actor_handle;
}

handle(rl_actor)
rl_get_rogue(struct rl_world const* world)
{
  return world->player.actor;
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

handle(rl_item)
rl_create_item(struct rl_world* world, enum rl_item_type type)
{
  if (pool_full(&world->items) &&
      !pool_reserve(&world->items, (size_t)pool_cap(&world->items) * 2)) {
    return handle_invalid(rl_item);
  }

  handle(rl_item) item_handle;
  struct rl_item* item = pool_acquire(&world->items, &item_handle);
  if (item == NULL) {
    return handle_invalid(rl_item);
  }

  *item = rl_make_item(type);
  item->handle = item_handle;

  return item_handle;
}

struct rl_item const*
rl_borrow_item(struct rl_world const* world, handle(rl_item) item_handle)
{
  return pool_get(&world->items, item_handle);
}

struct rl_item*
rl_borrow_mut_item(struct rl_world* world, handle(rl_item) item_handle)
{
  return pool_get(&world->items, item_handle);
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

handle(rl_item)
rl_find_item(struct rl_world const* world,
             struct rl_level const* level,
             SDL_Point pos)
{
  if (level == NULL) {
    return handle_invalid(rl_item);
  }

  for (size_t i = 0; i < alist_len(&level->items); i++) {
    handle(rl_item) const item_handle = *alist_at(&level->items, i);
    struct rl_item const* item = rl_borrow_item(world, item_handle);
    if (item == NULL) {
      // ignore stale handles
      continue;
    }

    if (item->on.map.x == pos.x && item->on.map.y == pos.y) {
      return item_handle;
    }
  }

  return handle_invalid(rl_item);
}

static bool
is_held_by(struct rl_item const* item, handle(rl_actor) holder)
{
  return item != NULL && item->ltype == RL_ITEM_LOCATION_HELD &&
         handle_equal(item->on.actor, holder);
}

int
rl_count_held_items(struct rl_world const* world, handle(rl_actor) holder)
{
  int count = 0;
  for (Uint32 i = 0; i < pool_cap(&world->items); i++) {
    if (is_held_by(pool_at_index(&world->items, i), holder)) {
      count++;
    }
  }

  return count;
}

handle(rl_item)
rl_find_held_item(struct rl_world const* world, handle(rl_actor) holder, int n)
{
  int seen = 0;
  for (Uint32 i = 0; i < pool_cap(&world->items); i++) {
    struct rl_item const* item = pool_at_index(&world->items, i);
    if (!is_held_by(item, holder)) {
      continue;
    }

    if (seen == n) {
      return item->handle;
    }
    seen++;
  }

  return handle_invalid(rl_item);
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
