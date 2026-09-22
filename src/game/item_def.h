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
  RL_ITEM_POTION_HEALTH,
  RL_ITEM_SCROLL_FIREBALL,
  RL_ITEM_SCROLL_LIGHTNING,
  RL_ITEM_WEAPON_DAGGER,
  RL_ITEM_WEAPON_SWORD,
  RL_ITEM_ARMOUR_LEATHER,
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
  /** The strength of the effect at level 1. */
  int power;
  /** The strength gained per level. */
  int power_per_level;
  /** Radius, in tiles, of the area a tile-targeted item affects. */
  int area_radius;
};

/**
 * Stat bonuses granted by an equipped item.
 */
struct rl_item_bonuses
{
  /** Added to the wearer's strength. */
  int strength;
  /** Added to the wearer's agility. */
  int agility;
  /** Added to the wearer's armour. */
  int armour;
};

/**
 * Immutable data that defines an equippable item.
 */
struct rl_item_equippable_def
{
  /** The bonuses at level 1. */
  struct rl_item_bonuses base;
  /** The bonuses gained per level. */
  struct rl_item_bonuses per_level;
};

/**
 * A qualifier given to items that reach a level.
 */
struct rl_item_tier
{
  /** The lowest level that earns this tier. */
  int min_level;
  /** The qualifier, such as "Minor". */
  char const* label;
};

/**
 * Immutable data that defines an item.
 *
 * @invariant tiers are in ascending min_level order and the first has a
 * min_level of 1.
 */
struct rl_item_def
{
  /** The category of the item. */
  enum rl_item_class class;
  /** The display name; a format with one %s for the tier. */
  char const* name;
  /** The tiers. */
  struct rl_item_tier const* tiers;
  /** The number of tiers. */
  int tier_count;

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

/**
 * @param level must be at least 1.
 *
 * @return the highest tier that level reaches for type.
 */
struct rl_item_tier const*
rl_get_item_tier(enum rl_item_type type, int level);

#endif // GINC_ROGUELIKE_ITEM_DEF_H
