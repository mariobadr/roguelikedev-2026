/**
 * @file item.h
 */
#ifndef GINC_ROGUELIKE_ITEM_H
#define GINC_ROGUELIKE_ITEM_H

#include <SDL3/SDL_rect.h>

#include "game/handles.h"
#include "game/item_def.h"

/** Where an item can be found. */
enum rl_item_location
{
  RL_ITEM_LOCATION_NONE, //< not placed yet
  RL_ITEM_LOCATION_MAP,  //< on a level's floor
  RL_ITEM_LOCATION_HELD, //< held by an actor
};

/** An instance of an item. */
struct rl_item
{
  /** This item's handle, so code holding a pointer can refer to it. */
  handle(rl_item) handle;
  /** For getting the item definition. */
  enum rl_item_type itype;
  /** For distinguishing where the item is found. */
  enum rl_item_location ltype;

  union
  {
    /** When ltype is map. */
    SDL_Point map;
    /** When ltype is held. */
    handle(rl_actor) actor;
  } on;
};

/**
 * Create a new, unplaced item. Its handle is invalid until the item is added
 * to a world (see rl_create_item).
 */
struct rl_item
rl_make_item(enum rl_item_type type);

#endif // GINC_ROGUELIKE_ITEM_H
