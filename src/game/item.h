/**
 * @file item.h
 */
#ifndef GINC_ROGUELIKE_ITEM_H
#define GINC_ROGUELIKE_ITEM_H

#include <SDL3/SDL_rect.h>

/** Item categories. */
enum rl_item_class
{
  RL_ITEM_CLASS_POTION,
};

/** Possible items found in the game. */
enum rl_item_type
{
  RL_ITEM_POTION_HEALTH_MINOR,
};

/** The different effects an item can have. */
enum rl_item_effect
{
  RL_ITEM_EFFECT_HEAL,
};

/** Immutable data that defines an item. */
struct rl_item_def
{
  enum rl_item_class class;
  char const* name;
  enum rl_item_effect effect;
  int power;
};

/** Where an item can be found. */
enum rl_item_location
{
  RL_ITEM_LOCATION_NONE,
  RL_ITEM_LOCATION_MAP,
  RL_ITEM_LOCATION_HELD,
};

/** An instance of an item. */
struct rl_item
{
  /** A unique identifier. */
  int id;
  /** For getting the item definition. */
  enum rl_item_type itype;
  /** For distinguishing where the item is found. */
  enum rl_item_location ltype;

  union
  {
    /** When ltype is map. */
    SDL_Point map;
    /** When ltype is held. */
    int actor;
  } on;
};

/**
 * @return the item definition that corresponds to type.
 */
struct rl_item_def const*
rl_get_item_def(enum rl_item_type type);

#endif // GINC_ROGUELIKE_ITEM_H
