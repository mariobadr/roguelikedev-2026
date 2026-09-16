/**
 * @file item.h
 */
#ifndef GINC_ROGUELIKE_ITEM_H
#define GINC_ROGUELIKE_ITEM_H

#include <SDL3/SDL_rect.h>

#include "game/handles.h"

/** Item categories. */
enum rl_item_class
{
  RL_ITEM_CLASS_POTION,
  RL_ITEM_CLASS_SCROLL,
};

/** Possible items found in the game. */
enum rl_item_type
{
  RL_ITEM_POTION_HEALTH_MINOR,
  RL_ITEM_SCROLL_FIREBALL,
  RL_ITEM_SCROLL_LIGHTNING,
};

/** The different effects an item can have. */
enum rl_item_effect
{
  RL_ITEM_EFFECT_HEAL,
  RL_ITEM_EFFECT_DAMAGE_AREA,
  RL_ITEM_EFFECT_DAMAGE_NEAREST,
};

/** The different targetting requirements of an item. */
enum rl_item_target
{
  RL_ITEM_TARGET_NONE,
  RL_ITEM_TARGET_TILE,
  /** Automatically targets the closest visible enemy. */
  RL_ITEM_TARGET_CLOSEST,
};

/** Immutable data that defines an item. */
struct rl_item_def
{
  enum rl_item_class class;
  enum rl_item_effect effect;
  enum rl_item_target target;

  char const* name;
  int power;
  /** Radius, in tiles, of the area a tile-targeted item affects (Euclidean). */
  int area_radius;
};

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

/**
 * @return the item definition that corresponds to type.
 */
struct rl_item_def const*
rl_get_item_def(enum rl_item_type type);

#endif // GINC_ROGUELIKE_ITEM_H
