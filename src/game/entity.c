#include "entity.h"

#include "procgen/rand.h"

#define MISS_CHANCE 5
#define ARMOR_SCALING 20

static struct rl_entity const entity_table[] = {
  [RL_ENTITY_ROGUE] = {
    .type = RL_ENTITY_ROGUE,
    .name = "Rogue",
    .max_hp = 20,
    .hp = 20,
    .strength = 6,
    .armor = 9,
  },
  [RL_ENTITY_RAT] = {
    .type = RL_ENTITY_RAT,
    .name = "Rat",
    .max_hp = 16,
    .hp = 16,
    .strength = 4,
    .armor = 5,
  },
};

struct rl_entity
rl_create_entity(enum rl_entity_type type, int id)
{
  struct rl_entity entity = entity_table[type];
  entity.id = id;

  return entity;
}

int
rl_attack_entity(struct rl_entity const* attacker,
                 struct rl_entity* defender,
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
