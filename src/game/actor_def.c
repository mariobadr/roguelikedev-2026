#include "actor_def.h"

static struct rl_actor_def const RL_ACTOR_DEFS[] = {
  [RL_ACTOR_ROGUE] = {
    .name = "Rogue",
    .max_hp = 20,
    .strength = 6,
    .armor = 9,
  },
  [RL_ACTOR_RAT] = {
    .name = "Rat",
    .max_hp = 16,
    .strength = 4,
    .armor = 5,
  },
};

struct rl_actor_def const*
rl_get_actor_def(enum rl_actor_type type)
{
  return &RL_ACTOR_DEFS[type];
}
