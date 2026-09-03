#include "actor.h"

#include "procgen/rand.h"

#define MISS_CHANCE 5
#define ARMOR_SCALING 20

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
rl_attack_actor(struct rl_actor const* attacker,
                struct rl_actor* defender,
                struct rand_state* rng)
{
  if (rand_next_up_to(rng, 100) < MISS_CHANCE) {
    return -1;
  }

  // from the good old WoW days
  int const ap = 2 * attacker->strength;
  // integer division truncates, but we avoid floating point (yay!)
  int const base = (int)rand_next_between(rng, ap * 8 / 10, ap * 12 / 10);
  // our random base damage is then mitigated by armor
  int const damage =
    base - (base * defender->armor / (defender->armor + ARMOR_SCALING));

  // don't let HP dip below 0
  defender->hp = SDL_max(0, defender->hp - damage);

  return damage;
}
