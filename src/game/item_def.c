#include "item_def.h"

#include <stddef.h>

static struct rl_item_def const RL_ITEM_DEFS[] = {
  [RL_ITEM_POTION_HEALTH_MINOR] = {
    .class = RL_ITEM_CLASS_POTION,
    .name = "Minor Health Potion",
    .as.consumable = {
      .effect = RL_ITEM_EFFECT_HEAL,
      .target = RL_ITEM_TARGET_NONE,
      .power = 10,
    },
  },
  [RL_ITEM_SCROLL_FIREBALL] = {
    .class = RL_ITEM_CLASS_SCROLL,
    .name = "Scroll of Minor Fireball",
    .as.consumable = {
      .effect = RL_ITEM_EFFECT_DAMAGE_AREA,
      .target = RL_ITEM_TARGET_TILE,
      .power = 8,
      .area_radius = 3,
    },
  },
  [RL_ITEM_SCROLL_LIGHTNING] = {
    .class = RL_ITEM_CLASS_SCROLL,
    .name = "Scroll of Minor Lightning",
    .as.consumable = {
      .effect = RL_ITEM_EFFECT_DAMAGE_NEAREST,
      .target = RL_ITEM_TARGET_CLOSEST,
      .power = 12,
    },
  },
  [RL_ITEM_WEAPON_DAGGER] = {
    .class = RL_ITEM_CLASS_WEAPON,
    .name = "Chipped Dagger",
    .as.equippable = {
      .strength_bonus = 2,
      .armour_bonus = 0,
    },
  },
  [RL_ITEM_WEAPON_SWORD] = {
    .class = RL_ITEM_CLASS_WEAPON,
    .name = "Chipped Sword",
    .as.equippable = {
      .strength_bonus = 4,
      .armour_bonus = 0,
    },
  },
  [RL_ITEM_ARMOUR_LEATHER] = {
    .class = RL_ITEM_CLASS_ARMOUR,
    .name = "Leather Armour",
    .as.equippable = {
      .strength_bonus = 0,
      .armour_bonus = 9,
    },
  },
  [RL_ITEM_ARMOUR_MAIL] = {
    .class = RL_ITEM_CLASS_ARMOUR,
    .name = "Chainmail Armour",
    .as.equippable = {
      .strength_bonus = 0,
      .armour_bonus = 20,
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
