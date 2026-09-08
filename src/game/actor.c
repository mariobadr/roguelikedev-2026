#include "actor.h"

static struct rl_actor const actor_table[] = {
  [RL_ACTOR_ROGUE] = {
    .type = RL_ACTOR_ROGUE,
    .name = "Rogue",
    .awake = true,
    .max_hp = 20,
    .hp = 20,
    .strength = 6,
    .armor = 9,
  },
  [RL_ACTOR_RAT] = {
    .type = RL_ACTOR_RAT,
    .name = "Rat",
    .awake = false,
    .max_hp = 16,
    .hp = 16,
    .strength = 4,
    .armor = 5,
  },
};

struct rl_actor
rl_create_actor(enum rl_actor_type type, int id)
{
  struct rl_actor actor = actor_table[type];
  actor.id = id;

  return actor;
}

int
rl_heal_actor(struct rl_actor* actor, int amount)
{
  int const old_hp = actor->hp;
  actor->hp = SDL_min(actor->max_hp, actor->hp + amount);

  return actor->hp - old_hp;
}
