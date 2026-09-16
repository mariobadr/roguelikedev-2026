/**
 * @file item_def.h
 */
#ifndef GINC_ROGUELIKE_ITEM_DEF_H
#define GINC_ROGUELIKE_ITEM_DEF_H

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

/**
 * @return the item definition that corresponds to type.
 */
struct rl_item_def const*
rl_get_item_def(enum rl_item_type type);

#endif // GINC_ROGUELIKE_ITEM_DEF_H
