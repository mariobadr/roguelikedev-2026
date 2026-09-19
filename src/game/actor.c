#include "actor.h"

#include "game/actor_def.h"

struct rl_actor
rl_make_actor(enum rl_actor_type type, int level)
{
  struct rl_actor actor = { 0 };
  actor.type = type;
  actor.level = level;
  actor.handle = handle_invalid(rl_actor);

  struct rl_actor_def const* def = rl_get_actor_def(type);
  int const growth = level - 1;
  actor.name = def->name;
  actor.max_hp = def->base_hp + growth * def->hp_per_level;
  actor.strength = def->base_strength + growth * def->strength_per_level;
  actor.armor = def->base_armor + growth * def->armor_per_level;
  actor.hp = actor.max_hp;

  return actor;
}

int
rl_heal_actor(struct rl_actor* actor, int amount)
{
  int const old_hp = actor->hp;
  actor->hp = SDL_min(actor->max_hp, actor->hp + amount);

  return actor->hp - old_hp;
}
