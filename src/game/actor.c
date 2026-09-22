#include "actor.h"

#include "game/actor_def.h"
#include "game/equipment.h"
#include "game/item.h"
#include "game/item_def.h"
#include "game/world.h"

struct rl_actor
rl_make_actor(enum rl_actor_type type, int level)
{
  struct rl_actor actor = { 0 };
  actor.type = type;
  actor.level = level;
  actor.handle = handle_invalid(rl_actor);

  rl_init_equipment(&actor.equipment);

  struct rl_actor_def const* def = rl_get_actor_def(type);
  int const growth = level - 1;
  actor.name = def->name;
  actor.stats.max_hp = def->base.max_hp + growth * def->per_level.max_hp;
  actor.stats.strength = def->base.strength + growth * def->per_level.strength;
  actor.stats.agility = def->base.agility + growth * def->per_level.agility;
  actor.stats.armor = def->base.armor + growth * def->per_level.armor;
  actor.hp = actor.stats.max_hp;

  return actor;
}

int
rl_heal_actor(struct rl_actor* actor, int amount)
{
  int const old_hp = actor->hp;
  actor->hp = SDL_min(actor->stats.max_hp, actor->hp + amount);

  return actor->hp - old_hp;
}

struct rl_actor_stats
rl_get_actor_stats(struct rl_world const* world, struct rl_actor const* actor)
{
  struct rl_actor_stats stats = actor->stats;
  for (int slot = 0; slot < RL_EQUIPMENT_SLOT_COUNT; ++slot) {
    struct rl_item const* item =
      rl_borrow_item(world, actor->equipment.slots[slot]);
    if (item == NULL) {
      continue;
    }

    struct rl_item_bonuses const bonuses = rl_get_item_bonuses(item);
    stats.strength += bonuses.strength;
    stats.agility += bonuses.agility;
    stats.armor += bonuses.armour;
  }

  return stats;
}
