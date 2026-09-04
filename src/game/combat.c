#include "combat.h"

#include "procgen/rand.h"

#include "actor.h"

#define MISS_CHANCE 5
#define ARMOR_SCALING 20

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
