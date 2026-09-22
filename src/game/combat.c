#include "combat.h"

#include "core/rand.h"

#include "actor.h"

#define MISS_CHANCE 5
#define ARMOR_SCALING 20
// in tenths of a percent
#define BASE_CRIT_CHANCE 50
#define CRIT_MULTIPLIER 2

struct rl_roll_range
rl_get_roll_range(int power)
{
  // integer division truncates, but we avoid floating point (yay!)
  return (struct rl_roll_range){
    .min = power * 8 / 10,
    .max = power * 12 / 10,
  };
}

int
rl_get_crit_chance(int agility, int level)
{
  // like WoW, each point of agility is worth less as the actor levels up
  return BASE_CRIT_CHANCE + agility * 100 / (level + 9);
}

struct rl_attack
rl_resolve_attack(struct rl_actor* defender,
                  int power,
                  int crit_chance,
                  int armour,
                  struct rand_state* rng)
{
  struct rl_attack attack = { 0 };

  if (rand_next_up_to(rng, 100) < MISS_CHANCE) {
    attack.damage = -1;
    return attack;
  }

  struct rl_roll_range const range = rl_get_roll_range(power);
  int base = (int)rand_next_between(rng, range.min, range.max);
  if (rand_next_up_to(rng, 1000) < (Uint64)crit_chance) {
    attack.critical = true;
    base *= CRIT_MULTIPLIER;
  }

  // our random base damage is then mitigated by armour
  attack.damage = base - (base * armour / (armour + ARMOR_SCALING));

  // don't let HP dip below 0
  defender->hp = SDL_max(0, defender->hp - attack.damage);

  return attack;
}
