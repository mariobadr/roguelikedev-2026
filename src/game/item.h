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
  RL_ITEM_LOCATION_NONE,     //< not placed yet
  RL_ITEM_LOCATION_MAP,      //< on a level's floor
  RL_ITEM_LOCATION_HELD,     //< held by an actor
  RL_ITEM_LOCATION_EQUIPPED, //< equipped on an actor
};

/**
 * An instance of an item.
 *
 * @invariant level is the min_level of one of its type's tiers.
 */
struct rl_item
{
  /** This item's handle, so code holding a pointer can refer to it. */
  handle(rl_item) handle;
  /** For getting the item definition. */
  enum rl_item_type itype;
  /** Scales the item's stats and picks its tier. */
  int level;
  /** For distinguishing where the item is found. */
  enum rl_item_location ltype;

  union
  {
    /** When ltype is map. */
    SDL_Point map;
    /** When ltype is held or equipped */
    handle(rl_actor) actor;
  } on;
};

/**
 * Create a new, unplaced item at the tier that level reaches. Its handle is
 * invalid until the item is added to a world (see rl_create_item).
 *
 * @param level must be at least 1.
 */
struct rl_item
rl_make_item(enum rl_item_type type, int level);

/**
 * @param item must be consumable.
 *
 * @return the strength of the item's effect at its level.
 */
int
rl_get_item_power(struct rl_item const* item);

/**
 * @param item must be equippable.
 *
 * @return the item's bonuses at its level.
 */
struct rl_item_bonuses
rl_get_item_bonuses(struct rl_item const* item);

/**
 * Write the item's display name, including its tier, into buf.
 */
void
rl_format_item_name(struct rl_item const* item, char* buf, size_t size);

#endif // GINC_ROGUELIKE_ITEM_H
