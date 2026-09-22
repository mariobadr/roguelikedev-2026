#include "item.h"

#include <SDL3/SDL_iostream.h>

#include "context.h"
#include "result.h"

#include "game/item.h"

bool
rl_is_valid_item_type(enum rl_item_type type)
{
  switch (type) {
    case RL_ITEM_POTION_HEALTH:
    case RL_ITEM_SCROLL_FIREBALL:
    case RL_ITEM_SCROLL_LIGHTNING:
    case RL_ITEM_WEAPON_DAGGER:
    case RL_ITEM_WEAPON_SWORD:
    case RL_ITEM_ARMOUR_LEATHER:
      return true;
    case RL_ITEM_TYPE_COUNT:
      break;
  }

  return false;
}

static bool
is_valid_item_location(enum rl_item_location type)
{
  switch (type) {
    case RL_ITEM_LOCATION_NONE:
    case RL_ITEM_LOCATION_MAP:
    case RL_ITEM_LOCATION_HELD:
    case RL_ITEM_LOCATION_EQUIPPED:
      return true;
  }

  return false;
}

bool
rl_write_item(SDL_IOStream* dst,
              struct rl_writer const* w,
              struct rl_item const* item)
{
  if (!rl_is_valid_item_type(item->itype) ||
      !is_valid_item_location(item->ltype)) {
    return false;
  }
  if (item->ltype == RL_ITEM_LOCATION_EQUIPPED &&
      rl_get_item_equippable_def(item->itype) == NULL) {
    return false;
  }

  bool ok = true;
  ok &= SDL_WriteU32LE(dst, (Uint32)item->itype);
  ok &= SDL_WriteS32LE(dst, (Sint32)item->level);
  ok &= SDL_WriteU32LE(dst, (Uint32)item->ltype);

  switch (item->ltype) {
    case RL_ITEM_LOCATION_MAP:
      ok &= SDL_WriteS32LE(dst, (Sint32)item->on.map.x);
      ok &= SDL_WriteS32LE(dst, (Sint32)item->on.map.y);
      break;
    case RL_ITEM_LOCATION_HELD:
    case RL_ITEM_LOCATION_EQUIPPED: {
      Uint32 const actor_id = rl_to_actor_id(w, item->on.actor);
      ok &= actor_id != 0;
      ok &= SDL_WriteU32LE(dst, actor_id);
      break;
    }
    case RL_ITEM_LOCATION_NONE:
      break;
  }

  return ok;
}

enum rl_read_result
rl_read_item(SDL_IOStream* src, struct rl_reader const* r, struct rl_item* out)
{
  Uint32 itype_value = 0;
  RL_READ_OR_FAIL(src, SDL_ReadU32LE(src, &itype_value));
  if (!rl_is_valid_item_type((enum rl_item_type)itype_value)) {
    return RL_READ_CORRUPT;
  }

  Sint32 level = 0;
  RL_READ_OR_FAIL(src, SDL_ReadS32LE(src, &level));
  if (level < 1 ||
      rl_get_item_tier((enum rl_item_type)itype_value, (int)level)->min_level !=
        (int)level) {
    return RL_READ_CORRUPT;
  }

  Uint32 ltype_value = 0;
  RL_READ_OR_FAIL(src, SDL_ReadU32LE(src, &ltype_value));
  if (!is_valid_item_location((enum rl_item_location)ltype_value)) {
    return RL_READ_CORRUPT;
  }

  out->itype = (enum rl_item_type)itype_value;
  out->level = (int)level;
  out->ltype = (enum rl_item_location)ltype_value;

  if (out->ltype == RL_ITEM_LOCATION_EQUIPPED &&
      rl_get_item_equippable_def(out->itype) == NULL) {
    return RL_READ_CORRUPT;
  }

  switch (out->ltype) {
    case RL_ITEM_LOCATION_MAP: {
      Sint32 x = 0;
      Sint32 y = 0;
      RL_READ_OR_FAIL(src, SDL_ReadS32LE(src, &x));
      RL_READ_OR_FAIL(src, SDL_ReadS32LE(src, &y));
      out->on.map.x = (int)x;
      out->on.map.y = (int)y;
      break;
    }
    case RL_ITEM_LOCATION_HELD:
    case RL_ITEM_LOCATION_EQUIPPED: {
      Uint32 actor_id = 0;
      RL_READ_OR_FAIL(src, SDL_ReadU32LE(src, &actor_id));
      if (actor_id < 1 || actor_id > r->actor_count) {
        return RL_READ_CORRUPT;
      }
      out->on.actor = *array_at(&r->actor_handles, actor_id);
      break;
    }
    case RL_ITEM_LOCATION_NONE:
      break;
  }

  return RL_READ_OK;
}
