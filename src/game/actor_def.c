#include "actor_def.h"

static struct rl_actor_def const RL_ACTOR_DEFS[] = {
  [RL_ACTOR_ROGUE] = {
    .class = RL_ACTOR_HUMANOID,
    .name = "Rogue",
    .base_hp = 30,
    .hp_per_level = 3,
    .base_strength = 6,
    .strength_per_level = 2,
    .base_armor = 9,
    .armor_per_level = 2,
  },
  [RL_ACTOR_RAT] = {
    .class = RL_ACTOR_BEAST,
    .name = "Rat",
    .base_hp = 16,
    .hp_per_level = 2,
    .base_strength = 4,
    .strength_per_level = 1,
    .base_armor = 5,
    .armor_per_level = 1,
  },
};

struct rl_actor_def const*
rl_get_actor_def(enum rl_actor_type type)
{
  return &RL_ACTOR_DEFS[type];
}
