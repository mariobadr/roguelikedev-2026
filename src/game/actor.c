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
  handle(rl_item) const slots[] = {
    actor->equipment.weapon,
    actor->equipment.armour,
  };

  for (size_t i = 0; i < SDL_arraysize(slots); i++) {
    struct rl_item const* item = rl_borrow_item(world, slots[i]);
    if (item == NULL || !rl_is_equipped_by(actor, item)) {
      continue;
    }

    struct rl_item_equippable_def const* def =
      rl_get_item_equippable_def(item->itype);
    if (def != NULL) {
      stats.strength += def->strength_bonus;
      stats.armor += def->armour_bonus;
    }
  }

  return stats;
}
