#include "item_def.h"

#include <stddef.h>

#include <SDL3/SDL_stdinc.h>

static struct rl_item_tier const CONSUMABLE_TIERS[] = {
  { .min_level = 1, .label = "Minor" },
  { .min_level = 3, .label = "Lesser" },
  { .min_level = 5, .label = "Greater" },
  { .min_level = 8, .label = "Major" },
};

static struct rl_item_tier const WEAPON_TIERS[] = {
  { .min_level = 1, .label = "Chipped" },
  { .min_level = 3, .label = "Worn" },
  { .min_level = 5, .label = "Fine" },
  { .min_level = 8, .label = "Masterwork" },
};

static struct rl_item_tier const ARMOUR_TIERS[] = {
  { .min_level = 1, .label = "Tattered" },
  { .min_level = 3, .label = "Worn" },
  { .min_level = 5, .label = "Sturdy" },
  { .min_level = 8, .label = "Masterwork" },
};

static struct rl_item_def const RL_ITEM_DEFS[] = {
  [RL_ITEM_POTION_HEALTH] = {
    .class = RL_ITEM_CLASS_POTION,
    .name = "%s Health Potion",
    .tiers = CONSUMABLE_TIERS,
    .tier_count = SDL_arraysize(CONSUMABLE_TIERS),
    .as.consumable = {
      .effect = RL_ITEM_EFFECT_HEAL,
      .target = RL_ITEM_TARGET_NONE,
      .power = 15,
      .power_per_level = 5,
    },
  },
  [RL_ITEM_SCROLL_FIREBALL] = {
    .class = RL_ITEM_CLASS_SCROLL,
    .name = "Scroll of %s Fireball",
    .tiers = CONSUMABLE_TIERS,
    .tier_count = SDL_arraysize(CONSUMABLE_TIERS),
    .as.consumable = {
      .effect = RL_ITEM_EFFECT_DAMAGE_AREA,
      .target = RL_ITEM_TARGET_TILE,
      .power = 16,
      .power_per_level = 5,
      .area_radius = 3,
    },
  },
  [RL_ITEM_SCROLL_LIGHTNING] = {
    .class = RL_ITEM_CLASS_SCROLL,
    .name = "Scroll of %s Lightning",
    .tiers = CONSUMABLE_TIERS,
    .tier_count = SDL_arraysize(CONSUMABLE_TIERS),
    .as.consumable = {
      .effect = RL_ITEM_EFFECT_DAMAGE_NEAREST,
      .target = RL_ITEM_TARGET_CLOSEST,
      .power = 24,
      .power_per_level = 6,
    },
  },
  [RL_ITEM_WEAPON_DAGGER] = {
    .class = RL_ITEM_CLASS_WEAPON,
    .name = "%s Dagger",
    .tiers = WEAPON_TIERS,
    .tier_count = SDL_arraysize(WEAPON_TIERS),
    .as.equippable = {
      .base = {
        .strength = 0,
        .agility = 2,
        .armour = 0,
      },
      .per_level = {
        .strength = 0,
        .agility = 2,
        .armour = 0,
      },
    },
  },
  [RL_ITEM_WEAPON_SWORD] = {
    .class = RL_ITEM_CLASS_WEAPON,
    .name = "%s Sword",
    .tiers = WEAPON_TIERS,
    .tier_count = SDL_arraysize(WEAPON_TIERS),
    .as.equippable = {
      .base = {
        .strength = 2,
        .agility = 0,
        .armour = 0,
      },
      .per_level = {
        .strength = 2,
        .agility = 0,
        .armour = 0,
      },
    },
  },
  [RL_ITEM_ARMOUR_LEATHER] = {
    .class = RL_ITEM_CLASS_ARMOUR,
    .name = "%s Armour",
    .tiers = ARMOUR_TIERS,
    .tier_count = SDL_arraysize(ARMOUR_TIERS),
    .as.equippable = {
      .base = {
        .strength = 0,
        .agility = 0,
        .armour = 9,
      },
      .per_level = {
        .strength = 0,
        .agility = 0,
        .armour = 2,
      },
    },
  },
};

struct rl_item_def const*
rl_get_item_def(enum rl_item_type type)
{
  return &RL_ITEM_DEFS[type];
}

struct rl_item_consumable_def const*
rl_get_item_consumable_def(enum rl_item_type type)
{
  struct rl_item_def const* def = rl_get_item_def(type);
  switch (def->class) {
    case RL_ITEM_CLASS_POTION:
    case RL_ITEM_CLASS_SCROLL:
      return &def->as.consumable;
    default:
      return NULL;
  }
}

struct rl_item_equippable_def const*
rl_get_item_equippable_def(enum rl_item_type type)
{
  struct rl_item_def const* def = rl_get_item_def(type);
  switch (def->class) {
    case RL_ITEM_CLASS_WEAPON:
    case RL_ITEM_CLASS_ARMOUR:
      return &def->as.equippable;
    default:
      return NULL;
  }
}

struct rl_item_tier const*
rl_get_item_tier(enum rl_item_type type, int level)
{
  struct rl_item_def const* def = rl_get_item_def(type);
  int i = 0;
  while (i + 1 < def->tier_count && level >= def->tiers[i + 1].min_level) {
    ++i;
  }

  return &def->tiers[i];
}
