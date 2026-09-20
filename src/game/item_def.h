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
  RL_ITEM_EFFECT_HEAL,           //< restore hit points
  RL_ITEM_EFFECT_DAMAGE_AREA,    //< damage actors in an area
  RL_ITEM_EFFECT_DAMAGE_NEAREST, //< damage the nearest enemy
};

/** The different targetting requirements of an item. */
enum rl_item_target
{
  RL_ITEM_TARGET_NONE,    //< no target
  RL_ITEM_TARGET_TILE,    //< a chosen tile
  RL_ITEM_TARGET_CLOSEST, //< closest visible enemy
};

/**
 * Immutable data that defines an item.
 */
struct rl_item_def
{
  /** The category of the item. */
  enum rl_item_class class;
  /** What the item does when used. */
  enum rl_item_effect effect;
  /** What the item must be aimed at, if anything. */
  enum rl_item_target target;

  /** The display name. */
  char const* name;
  /** The strength of the effect, such as the healing or damage done. */
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
