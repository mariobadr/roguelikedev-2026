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
  RL_ITEM_CLASS_WEAPON,
  RL_ITEM_CLASS_ARMOUR,
};

/** Possible items found in the game. */
enum rl_item_type
{
  RL_ITEM_POTION_HEALTH_MINOR,
  RL_ITEM_SCROLL_FIREBALL,
  RL_ITEM_SCROLL_LIGHTNING,
  RL_ITEM_WEAPON_DAGGER,
  RL_ITEM_WEAPON_SWORD,
  RL_ITEM_ARMOUR_LEATHER,
  RL_ITEM_ARMOUR_MAIL,
  RL_ITEM_TYPE_COUNT, //< number of item types; not an item
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
 * Immutable data that defines a consumable item.
 */
struct rl_item_consumable_def
{
  /** What the item does when consumed. */
  enum rl_item_effect effect;
  /** What the item must be aimed at, if anything. */
  enum rl_item_target target;
  /** The strength of the effect. */
  int power;
  /** Radius, in tiles, of the area a tile-targeted item affects. */
  int area_radius;
};

/**
 * Immutable data that defines an equippable item.
 */
struct rl_item_equippable_def
{
  int strength_bonus;
  int armour_bonus;
};

/**
 * Immutable data that defines an item.
 */
struct rl_item_def
{
  /** The category of the item. */
  enum rl_item_class class;
  /** The display name. */
  char const* name;

  union
  {
    struct rl_item_consumable_def consumable;
    struct rl_item_equippable_def equippable;
  } as;
};

/**
 * @return the item definition that corresponds to type.
 */
struct rl_item_def const*
rl_get_item_def(enum rl_item_type type);

/**
 * @return the consumable definition, or NULL if type is not consumable.
 */
struct rl_item_consumable_def const*
rl_get_item_consumable_def(enum rl_item_type type);

/**
 * @return the equippable definition, or NULL if type is not equippable.
 */
struct rl_item_equippable_def const*
rl_get_item_equippable_def(enum rl_item_type type);

#endif // GINC_ROGUELIKE_ITEM_DEF_H
