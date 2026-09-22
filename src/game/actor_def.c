#include "actor_def.h"

#include "item_def.h"

static struct rl_loot_entry const RL_RAT_LOOT[] = {
  { .item = RL_ITEM_POTION_HEALTH, .weight = 8 },
  { .item = RL_ITEM_SCROLL_FIREBALL, .weight = 5 },
  { .item = RL_ITEM_SCROLL_LIGHTNING, .weight = 5 },
  { .item = RL_ITEM_WEAPON_DAGGER, .weight = 1 },
  { .item = RL_ITEM_WEAPON_SWORD, .weight = 1 },
  { .item = RL_ITEM_ARMOUR_LEATHER, .weight = 0 },
};

static struct rl_loot_entry const RL_GOBLIN_LOOT[] = {
  { .item = RL_ITEM_POTION_HEALTH, .weight = 5 },
  { .item = RL_ITEM_SCROLL_FIREBALL, .weight = 0 },
  { .item = RL_ITEM_SCROLL_LIGHTNING, .weight = 0 },
  { .item = RL_ITEM_WEAPON_DAGGER, .weight = 4 },
  { .item = RL_ITEM_WEAPON_SWORD, .weight = 4 },
  { .item = RL_ITEM_ARMOUR_LEATHER, .weight = 7 },
};

static struct rl_loot_entry const RL_TROLL_LOOT[] = {
  { .item = RL_ITEM_POTION_HEALTH, .weight = 5 },
  { .item = RL_ITEM_SCROLL_FIREBALL, .weight = 0 },
  { .item = RL_ITEM_SCROLL_LIGHTNING, .weight = 0 },
  { .item = RL_ITEM_WEAPON_DAGGER, .weight = 5 },
  { .item = RL_ITEM_WEAPON_SWORD, .weight = 5 },
  { .item = RL_ITEM_ARMOUR_LEATHER, .weight = 5 },
};

static struct rl_loot_entry const RL_BOSS_LOOT[] = {
  { .item = RL_ITEM_POTION_HEALTH, .weight = 0 },
  { .item = RL_ITEM_SCROLL_FIREBALL, .weight = 0 },
  { .item = RL_ITEM_SCROLL_LIGHTNING, .weight = 0 },
  { .item = RL_ITEM_WEAPON_SWORD, .weight = 10 },
  { .item = RL_ITEM_ARMOUR_LEATHER, .weight = 10 },
};

static struct rl_actor_def const RL_ACTOR_DEFS[] = {
  [RL_ACTOR_ROGUE] = {
    .class = RL_ACTOR_HUMANOID,
    .name = "Rogue",
    .base = {
      .max_hp = 30,
      .strength = 6,
      .agility = 4,
      .armor = 0,
    },
    .per_level = {
      .max_hp = 3,
      .strength = 2,
      .agility = 1,
      .armor = 0,
    },
    .loot = { 0 },
  },
  [RL_ACTOR_RAT] = {
    .class = RL_ACTOR_BEAST,
    .name = "Rat",
    .base = {
      .max_hp = 20,
      .strength = 4,
      .armor = 3,
    },
    .per_level = {
      .max_hp = 4,
      .strength = 1,
      .armor = 1,
    },
    .loot = {
      .drop_percent = 40,
      .items = RL_RAT_LOOT,
      .count = SDL_arraysize(RL_RAT_LOOT),
      .level_bonus = 0,
    },
  },
  [RL_ACTOR_GOBLIN] = {
    .class = RL_ACTOR_HUMANOID,
    .name = "Goblin",
    .base = {
      .max_hp = 30,
      .strength = 5,
      .armor = 4,
    },
    .per_level = {
      .max_hp = 5,
      .strength = 1,
      .armor = 2,
    },
    .loot = {
      .drop_percent = 45,
      .items = RL_GOBLIN_LOOT,
      .count = SDL_arraysize(RL_GOBLIN_LOOT),
      .level_bonus = 1,
    },
  },
  [RL_ACTOR_TROLL] = {
    .class = RL_ACTOR_HUMANOID,
    .name = "Troll",
    .base = {
      .max_hp = 40,
      .strength = 8,
      .armor = 5,
    },
    .per_level = {
      .max_hp = 6,
      .strength = 2,
      .armor = 2,
    },
    .loot = {
      .drop_percent = 50,
      .items = RL_TROLL_LOOT,
      .count = SDL_arraysize(RL_TROLL_LOOT),
      .level_bonus = 2,
    },
  },
  [RL_ACTOR_RAT_KING] = {
    .class = RL_ACTOR_BEAST,
    .name = "Rat King",
    .base = {
      .max_hp = 50,
      .strength = 8,
      .armor = 8,
    },
    .per_level = {
      .max_hp = 6,
      .strength = 2,
      .armor = 2,
    },
    .loot = {
      .drop_percent = 100,
      .items = RL_BOSS_LOOT,
      .count = SDL_arraysize(RL_BOSS_LOOT),
      .level_bonus = 3,
    },
  },
  [RL_ACTOR_GOBLIN_CHIEF] = {
    .class = RL_ACTOR_HUMANOID,
    .name = "Goblin Chief",
    .base = {
      .max_hp = 70,
      .strength = 8,
      .armor = 8,
    },
    .per_level = {
      .max_hp = 6,
      .strength = 2,
      .armor = 3,
    },
    .loot = {
      .drop_percent = 100,
      .items = RL_BOSS_LOOT,
      .count = SDL_arraysize(RL_BOSS_LOOT),
      .level_bonus = 3,
    },
  },
  [RL_ACTOR_TROLL_WARLORD] = {
    .class = RL_ACTOR_HUMANOID,
    .name = "Troll Warlord",
    .base = {
      .max_hp = 90,
      .strength = 8,
      .armor = 8,
    },
    .per_level = {
      .max_hp = 6,
      .strength = 3,
      .armor = 3,
    },
    .loot = {
      .drop_percent = 100,
      .items = RL_BOSS_LOOT,
      .count = SDL_arraysize(RL_BOSS_LOOT),
      .level_bonus = 3,
    },
  },
  [RL_ACTOR_DRAGON] = {
    .class = RL_ACTOR_BEAST,
    .name = "Dragon",
    .base = {
      .max_hp = 100,
      .strength = 10,
      .armor = 10,
    },
    .per_level = {
      .max_hp = 8,
      .strength = 4,
      .armor = 4,
    },
    .loot = { 0 },
  },
};

struct rl_actor_def const*
rl_get_actor_def(enum rl_actor_type type)
{
  return &RL_ACTOR_DEFS[type];
}
