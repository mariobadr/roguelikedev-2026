#include "level.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

bool
rl_alloc_level(struct rl_level* level, int depth, int width, int height)
{
  if (!grid_alloc(&level->map, width, height)) {
    SDL_Log("grid_alloc failed: %s", SDL_GetError());
    return false;
  }

  if (!grid_alloc(&level->explored, width, height)) {
    SDL_Log("grid_alloc failed: %s", SDL_GetError());
    return false;
  }

  if (!alist_alloc(&level->actors, 16)) {
    SDL_Log("alist_alloc failed: %s", SDL_GetError());
    return false;
  }

  if (!alist_alloc(&level->items, 8)) {
    SDL_Log("alist_alloc failed: %s", SDL_GetError());
    return false;
  }

  level->depth = depth;

  return true;
}

void
rl_free_level(struct rl_level* level)
{
  if (level == NULL) {
    return;
  }

  alist_free(&level->items);
  alist_free(&level->actors);
  grid_free(&level->explored);
  grid_free(&level->map);
}

bool
rl_add_actor(struct rl_level* level, handle(rl_actor) actor)
{
  for (size_t i = 0; i < alist_len(&level->actors); i++) {
    // a duplicate handle would give the actor more than one turn
    SDL_assert(!handle_equal(*alist_at(&level->actors, i), actor));
  }

  handle(rl_actor)* entry = alist_push(&level->actors);
  if (entry == NULL) {
    return false;
  }

  *entry = actor;
  return true;
}

bool
rl_add_item(struct rl_level* level, handle(rl_item) item)
{
  handle(rl_item)* entry = alist_push(&level->items);
  if (entry == NULL) {
    return false;
  }

  *entry = item;
  return true;
}

bool
rl_remove_item(struct rl_level* level, handle(rl_item) item)
{
  for (size_t i = 0; i < alist_len(&level->items); ++i) {
    if (handle_equal(*alist_at(&level->items, i), item)) {
      *alist_at(&level->items, i) = *alist_pop(&level->items);
      return true;
    }
  }

  return false;
}
