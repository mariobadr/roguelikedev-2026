#include "item.h"

#include <SDL3/SDL_stdinc.h>

struct rl_item
rl_make_item(enum rl_item_type type, int level)
{
  struct rl_item item = { 0 };
  item.handle = handle_invalid(rl_item);
  item.itype = type;
  item.level = rl_get_item_tier(type, level)->min_level;
  item.ltype = RL_ITEM_LOCATION_NONE;

  return item;
}

int
rl_get_item_power(struct rl_item const* item)
{
  struct rl_item_consumable_def const* def =
    rl_get_item_consumable_def(item->itype);
  int const growth = item->level - 1;

  return def->power + growth * def->power_per_level;
}

struct rl_item_bonuses
rl_get_item_bonuses(struct rl_item const* item)
{
  struct rl_item_equippable_def const* def =
    rl_get_item_equippable_def(item->itype);
  int const growth = item->level - 1;

  return (struct rl_item_bonuses){
    .strength = def->base.strength + growth * def->per_level.strength,
    .agility = def->base.agility + growth * def->per_level.agility,
    .armour = def->base.armour + growth * def->per_level.armour,
  };
}

void
rl_format_item_name(struct rl_item const* item, char* buf, size_t size)
{
  struct rl_item_def const* def = rl_get_item_def(item->itype);
  struct rl_item_tier const* tier = rl_get_item_tier(item->itype, item->level);

  SDL_snprintf(buf, size, def->name, tier->label);
}
