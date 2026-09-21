/**
 * @file loot.h
 */
#ifndef GINC_ROGUELIKE_LOOT_H
#define GINC_ROGUELIKE_LOOT_H

#include <SDL3/SDL_stdinc.h>

#include "game/item_def.h"

// forward declarations
struct rand_state;

/** An entry within a loot table. */
struct rl_loot_entry
{
  /** The item to be looted. */
  enum rl_item_type item;
  /** Relative weight; zero disables this entry. */
  Uint32 weight;
};

/**
 * Immutable data that defines a set of items that may be selected.
 */
struct rl_loot_table
{
  /** The chance, from 0 to 100, that a roll selects an item. */
  int drop_percent;
  /** The items that may be selected, in proportion to their weights. */
  struct rl_loot_entry const* items;
  /** The number of entries in items. */
  size_t count;
};

/**
 * Roll a table for an item.
 *
 * First roll drop_percent, then choose one item in proportion to its weight.
 * Weights need not total 100; they apply only when the table produces a drop.
 * 
 * An empty table or a drop_percent of 0 selects nothing and does not draw
 * from rng.
 *
 * @param table if non-empty, must have at least one non-zero weight.
 * @param out written only if an item is selected.
 *
 * @return whether an item was selected.
 */
bool
rl_roll_loot(struct rl_loot_table const* table,
             struct rand_state* rng,
             enum rl_item_type* out);

#endif // GINC_ROGUELIKE_LOOT_H
