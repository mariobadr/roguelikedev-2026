#include "item.h"

#include <SDL3/SDL_iostream.h>

#include "result.h"
#include "context.h"

#include "game/item.h"

static bool
is_valid_item_type(enum rl_item_type type)
{
  switch (type) {
    case RL_ITEM_POTION_HEALTH_MINOR:
    case RL_ITEM_SCROLL_FIREBALL:
    case RL_ITEM_SCROLL_LIGHTNING:
      return true;
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
      return true;
  }

  return false;
}

bool
rl_write_item(SDL_IOStream* dst,
              struct rl_writer const* w,
              struct rl_item const* item)
{
  bool ok = true;
  ok &= SDL_WriteU32LE(dst, (Uint32)item->itype);
  ok &= SDL_WriteU32LE(dst, (Uint32)item->ltype);

  switch (item->ltype) {
    case RL_ITEM_LOCATION_MAP:
      ok &= SDL_WriteS32LE(dst, (Sint32)item->on.map.x);
      ok &= SDL_WriteS32LE(dst, (Sint32)item->on.map.y);
      break;
    case RL_ITEM_LOCATION_HELD: {
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
  if (!is_valid_item_type((enum rl_item_type)itype_value)) {
    return RL_READ_CORRUPT;
  }

  Uint32 ltype_value = 0;
  RL_READ_OR_FAIL(src, SDL_ReadU32LE(src, &ltype_value));
  if (!is_valid_item_location((enum rl_item_location)ltype_value)) {
    return RL_READ_CORRUPT;
  }

  out->itype = (enum rl_item_type)itype_value;
  out->ltype = (enum rl_item_location)ltype_value;

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
    case RL_ITEM_LOCATION_HELD: {
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
