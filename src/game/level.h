/**
 * @file level.h
 */
#ifndef GINC_ROGUELIKE_LEVEL_H
#define GINC_ROGUELIKE_LEVEL_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "container/alist.h"

#include "game/handles.h"
#include "game/tile.h"

/**
 * A growable array of actor handles.
 */
alist_define_as(handle(rl_actor), rl_actor_handle);

/**
 * A growable array of item handles.
 */
alist_define_as(handle(rl_item), rl_item_handle);

struct rl_level
{
  /** How deep this level is in the overall dungeon. */
  int depth;
  /** A map of the current level. */
  grid(rl_tile) map;
  /** Cells the player has seen before */
  grid(boolean) explored;
  /** Actors on this level, in turn order. */
  alist(rl_actor_handle) actors;
  /** Items on this level's floor. */
  alist(rl_item_handle) items;
};

/**
 * A growable array of levels.
 */
alist_define_as(struct rl_level, rl_level);

bool
rl_alloc_level(struct rl_level* level, int depth, int width, int height);

void
rl_free_level(struct rl_level* level);

/**
 * Append actor to the end of level's turn order. The actor must not already be
 * on this level.
 *
 * @return whether the actor was added.
 */
bool
rl_add_actor(struct rl_level* level, handle(rl_actor) actor);

/**
 * Add item to level's floor. The item must not already be on this level.
 *
 * @return whether the item was added.
 */
bool
rl_add_item(struct rl_level* level, handle(rl_item) item);

/**
 * Remove item from level's floor. The order of the remaining items is
 * unspecified.
 *
 * @return whether the item was on this level.
 */
bool
rl_remove_item(struct rl_level* level, handle(rl_item) item);

/**
 * @return whether p has been explored.
 */
static inline bool
rl_is_tile_explored(struct rl_level const* level, SDL_Point p)
{
  if (!grid_contains(&level->map, p.x, p.y)) {
    return false;
  }

  return *grid_at(&level->explored, p.x, p.y);
}

#endif // GINC_ROGUELIKE_LEVEL_H
