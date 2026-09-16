#include "actor.h"

#include "game/actor_def.h"

struct rl_actor
rl_make_actor(enum rl_actor_type type)
{
  struct rl_actor actor = { 0 };
  actor.type = type;
  actor.handle = handle_invalid(rl_actor);

  struct rl_actor_def const* def = rl_get_actor_def(type);
  actor.name = def->name;
  actor.max_hp = def->max_hp;
  actor.strength = def->strength;
  actor.armor = def->armor;
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
