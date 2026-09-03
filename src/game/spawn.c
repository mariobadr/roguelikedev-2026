#include "spawn.h"

#include "procgen/rand.h"

#include "actor.h"

int
rl_gen_total_actors(int depth, struct rand_state* rng)
{
  int const min = 7 + depth;
  int const max = 14 + depth * 2;

  return (int)rand_next_between(rng, min, max);
}

enum rl_actor_type
rl_gen_actor_type(int depth, struct rand_state* rng)
{
  (void)depth;
  (void)rng;

  // TODO: fix when there are more types
  return RL_ACTOR_RAT;
}

int
rl_gen_total_items(struct rand_state* rng)
{
  return (int)rand_next_between(rng, 4, 10);
}

enum rl_item_type
rl_gen_item_type(int depth, struct rand_state* rng)
{
  (void)depth;
  (void)rng;

  // TODO: fix when there are more types
  return RL_ITEM_POTION_HEALTH_MINOR;
}
