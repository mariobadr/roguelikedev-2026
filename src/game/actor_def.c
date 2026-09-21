#include "actor_def.h"

#include "item_def.h"

static struct rl_loot_entry const RL_RAT_LOOT[] = {
  { .item = RL_ITEM_POTION_HEALTH_MINOR, .weight = 8 },
  { .item = RL_ITEM_SCROLL_FIREBALL, .weight = 5 },
  { .item = RL_ITEM_SCROLL_LIGHTNING, .weight = 5 },
  { .item = RL_ITEM_WEAPON_SWORD, .weight = 1 },
  { .item = RL_ITEM_ARMOUR_MAIL, .weight = 1 },
};

static struct rl_actor_def const RL_ACTOR_DEFS[] = {
  [RL_ACTOR_ROGUE] = {
    .class = RL_ACTOR_HUMANOID,
    .name = "Rogue",
    .base = {
      .max_hp = 30,
      .strength = 6,
      .armor = 0,
    },
    .per_level = {
      .max_hp = 3,
      .strength = 2,
      .armor = 0,
    },
    .loot = { 0 },
  },
  [RL_ACTOR_RAT] = {
    .class = RL_ACTOR_BEAST,
    .name = "Rat",
    .base = {
      .max_hp = 16,
      .strength = 4,
      .armor = 3,
    },
    .per_level = {
      .max_hp = 2,
      .strength = 1,
      .armor = 1,
    },
    .loot = {
      .drop_percent = 20,
      .items = RL_RAT_LOOT,
      .count = SDL_arraysize(RL_RAT_LOOT),
    },
  },
};

struct rl_actor_def const*
rl_get_actor_def(enum rl_actor_type type)
{
  return &RL_ACTOR_DEFS[type];
}
