#include "world.h"

#include <SDL3/SDL_iostream.h>

#include "context.h"
#include "result.h"

#include "actor.h"
#include "item.h"
#include "level.h"

#include "container/pool.h"

#include "game/equipment.h"
#include "game/experience.h"
#include "game/world.h"

static bool
write_actors(SDL_IOStream* dst,
             struct rl_world const* world,
             struct rl_writer const* w)
{
  bool ok = SDL_WriteU32LE(dst, w->actor_count);

  for (int i = 0; i < (int)array_cap(&w->actor_entries); i++) {
    if (array_at(&w->actor_entries, i)->id == 0) {
      continue;
    }

    struct rl_actor const* actor = pool_at_index(&world->actors, i);
    ok &= rl_write_actor(dst, actor);
  }

  return ok;
}

static bool
write_items(SDL_IOStream* dst,
            struct rl_world const* world,
            struct rl_writer const* w)
{
  bool ok = SDL_WriteU32LE(dst, w->item_count);

  for (int i = 0; i < (int)array_cap(&w->item_entries); i++) {
    if (array_at(&w->item_entries, i)->id == 0) {
      continue;
    }

    struct rl_item const* item = pool_at_index(&world->items, i);
    ok &= rl_write_item(dst, w, item);
  }

  return ok;
}

bool
rl_write_world(SDL_IOStream* dst, struct rl_world const* world)
{
  struct rl_writer w = { 0 };
  if (!rl_alloc_writer(world, &w)) {
    return false;
  }

  bool ok = write_actors(dst, world, &w);
  ok &= write_items(dst, world, &w);

  Uint32 const rogue_id = rl_to_actor_id(&w, world->player.actor);
  ok &= rogue_id != 0;
  ok &= SDL_WriteU32LE(dst, rogue_id);
  ok &= SDL_WriteS32LE(dst, (Sint32)world->player.xp);

  Uint32 const level_count = (Uint32)alist_len(&world->levels);
  ok &= SDL_WriteU32LE(dst, level_count);
  ok &= SDL_WriteS32LE(dst, (Sint32)world->current_level);

  for (Uint32 i = 0; i < level_count; i++) {
    ok &= rl_write_level(dst, &w, alist_at(&world->levels, i));
  }

  rl_free_writer(&w);

  return ok;
}

static enum rl_read_result
read_actors(SDL_IOStream* src, struct rl_world* world, struct rl_reader* r)
{
  Uint32 actor_count = 0;
  RL_READ_OR_FAIL(src, SDL_ReadU32LE(src, &actor_count));
  if (actor_count > RL_SNAPSHOT_MAX_ACTORS) {
    return RL_READ_CORRUPT;
  }

  r->actor_count = actor_count;

  if (actor_count > 0 &&
      !array_alloc(&r->actor_handles, (size_t)actor_count + 1)) {
    return RL_READ_ERROR;
  }

  for (Uint32 id = 1; id <= actor_count; id++) {
    struct rl_actor tmp = { 0 };
    enum rl_read_result const result = rl_read_actor(src, &tmp);
    if (result != RL_READ_OK) {
      return result;
    }

    handle(rl_actor) const h = rl_create_actor(world, tmp.type, tmp.level);
    struct rl_actor* actor = rl_borrow_mut_actor(world, h);
    if (actor == NULL) {
      return RL_READ_ERROR;
    }

    actor->pos = tmp.pos;
    actor->awake = tmp.awake;
    actor->hp = tmp.hp;
    actor->stats = tmp.stats;

    *array_at(&r->actor_handles, id) = h;
  }

  return RL_READ_OK;
}

static enum rl_read_result
restore_equipment(struct rl_world* world, struct rl_item* item)
{
  if (item->ltype != RL_ITEM_LOCATION_EQUIPPED) {
    return RL_READ_OK;
  }

  struct rl_actor* actor = rl_borrow_mut_actor(world, item->on.actor);
  if (actor == NULL) {
    return RL_READ_CORRUPT;
  }

  enum rl_equipment_slot const slot = rl_get_equipment_slot(item->itype);
  if (slot == RL_EQUIPMENT_SLOT_NONE ||
      handle_is_nonnull(rl_get_equipped_item(actor, slot))) {
    // the item is not equippable, or two items claim one slot
    return RL_READ_CORRUPT;
  }

  if (!rl_equip(actor, item)) {
    return RL_READ_CORRUPT;
  }

  return RL_READ_OK;
}

static enum rl_read_result
read_items(SDL_IOStream* src, struct rl_world* world, struct rl_reader* r)
{
  Uint32 item_count = 0;
  RL_READ_OR_FAIL(src, SDL_ReadU32LE(src, &item_count));
  if (item_count > RL_SNAPSHOT_MAX_ITEMS) {
    return RL_READ_CORRUPT;
  }

  r->item_count = item_count;

  if (item_count > 0 &&
      !array_alloc(&r->item_handles, (size_t)item_count + 1)) {
    return RL_READ_ERROR;
  }

  for (Uint32 id = 1; id <= item_count; id++) {
    struct rl_item tmp = { 0 };
    enum rl_read_result const result = rl_read_item(src, r, &tmp);
    if (result != RL_READ_OK) {
      return result;
    }

    handle(rl_item) const h = rl_create_item(world, tmp.itype);
    struct rl_item* item = rl_borrow_mut_item(world, h);
    if (item == NULL) {
      return RL_READ_ERROR;
    }

    item->ltype = tmp.ltype;
    item->on = tmp.on;

    enum rl_read_result const equipment_result = restore_equipment(world, item);
    if (equipment_result != RL_READ_OK) {
      return equipment_result;
    }

    *array_at(&r->item_handles, id) = h;
  }

  return RL_READ_OK;
}

static enum rl_read_result
read_entities(SDL_IOStream* src, struct rl_world* world, struct rl_reader* r)
{
  enum rl_read_result const result = read_actors(src, world, r);
  if (result != RL_READ_OK) {
    return result;
  }

  return read_items(src, world, r);
}

static enum rl_read_result
read_world_header(SDL_IOStream* src,
                  struct rl_world* world,
                  struct rl_reader const* r,
                  Uint32* out_level_count)
{
  Uint32 rogue_id = 0;
  RL_READ_OR_FAIL(src, SDL_ReadU32LE(src, &rogue_id));
  if (rogue_id < 1 || rogue_id > r->actor_count) {
    return RL_READ_CORRUPT;
  }

  Sint32 xp = 0;
  RL_READ_OR_FAIL(src, SDL_ReadS32LE(src, &xp));
  struct rl_actor const* rogue =
    rl_borrow_actor(world, *array_at(&r->actor_handles, rogue_id));
  if (xp < 0 || xp >= rl_xp_required(rogue->level)) {
    return RL_READ_CORRUPT;
  }

  Uint32 level_count = 0;
  RL_READ_OR_FAIL(src, SDL_ReadU32LE(src, &level_count));
  if (level_count > RL_SNAPSHOT_MAX_LEVELS) {
    return RL_READ_CORRUPT;
  }

  Sint32 current_level = 0;
  RL_READ_OR_FAIL(src, SDL_ReadS32LE(src, &current_level));
  if (current_level < 0 || (Uint32)current_level >= level_count) {
    return RL_READ_CORRUPT;
  }

  world->player.actor = *array_at(&r->actor_handles, rogue_id);
  world->player.xp = (int)xp;
  world->current_level = (int)current_level;

  *out_level_count = level_count;
  return RL_READ_OK;
}

static enum rl_read_result
validate_level(struct rl_world const* world,
               struct rl_level const* level,
               array(int) * actor_level,
               array(boolean) * item_listed,
               int level_index)
{
  for (size_t i = 0; i < alist_len(&level->actors); i++) {
    handle(rl_actor) const h = *alist_at(&level->actors, i);
    struct rl_actor const* actor = rl_borrow_actor(world, h);
    if (actor == NULL ||
        !grid_contains(&level->map, actor->pos.x, actor->pos.y)) {
      return RL_READ_CORRUPT;
    }

    if (*array_at(actor_level, h.index) != -1) {
      return RL_READ_CORRUPT;
    }

    *array_at(actor_level, h.index) = level_index;
  }

  for (size_t i = 0; i < alist_len(&level->items); i++) {
    handle(rl_item) const h = *alist_at(&level->items, i);
    struct rl_item const* item = rl_borrow_item(world, h);
    if (item == NULL || item->ltype != RL_ITEM_LOCATION_MAP ||
        !grid_contains(&level->map, item->on.map.x, item->on.map.y)) {
      return RL_READ_CORRUPT;
    }

    if (*array_at(item_listed, h.index)) {
      return RL_READ_CORRUPT;
    }

    *array_at(item_listed, h.index) = true;
  }

  return RL_READ_OK;
}

static enum rl_read_result
validate_world_references(struct rl_world const* world,
                          struct rl_reader const* r,
                          array(int) const* actor_level,
                          array(boolean) const* item_listed)
{
  for (Uint32 id = 1; id <= r->item_count; id++) {
    handle(rl_item) const h = *array_at(&r->item_handles, id);
    struct rl_item const* item = rl_borrow_item(world, h);
    if (item->ltype == RL_ITEM_LOCATION_MAP &&
        !*array_at(item_listed, h.index)) {
      return RL_READ_CORRUPT;
    }
  }

  if (*array_at(actor_level, world->player.actor.index) !=
      world->current_level) {
    return RL_READ_CORRUPT;
  }

  return RL_READ_OK;
}

static enum rl_read_result
read_levels(SDL_IOStream* src, struct rl_world* world, struct rl_reader* r)
{
  Uint32 level_count = 0;
  enum rl_read_result result = read_world_header(src, world, r, &level_count);
  if (result != RL_READ_OK) {
    return result;
  }

  Uint32 const actor_cap = pool_cap(&world->actors);
  array(int) actor_level = { 0 };
  if (actor_cap > 0) {
    if (!array_alloc(&actor_level, actor_cap)) {
      return RL_READ_ERROR;
    }

    for (Uint32 i = 0; i < actor_cap; i++) {
      *array_at(&actor_level, i) = -1;
    }
  }

  Uint32 const item_cap = pool_cap(&world->items);
  array(boolean) item_listed = { 0 };
  if (item_cap > 0 && !array_alloc(&item_listed, item_cap)) {
    array_free(&actor_level);
    return RL_READ_ERROR;
  }

  for (Uint32 i = 0; i < level_count; i++) {
    struct rl_level level_tmp = { 0 };

    result = rl_read_level(src, r, &level_tmp);
    if (result == RL_READ_OK) {
      result =
        validate_level(world, &level_tmp, &actor_level, &item_listed, (int)i);
    }

    if (result != RL_READ_OK) {
      rl_free_level(&level_tmp);
      break;
    }

    struct rl_level* slot = alist_push(&world->levels);
    if (slot == NULL) {
      rl_free_level(&level_tmp);
      result = RL_READ_ERROR;
      break;
    }

    *slot = level_tmp;
  }

  if (result == RL_READ_OK) {
    result = validate_world_references(world, r, &actor_level, &item_listed);
  }

  array_free(&actor_level);
  array_free(&item_listed);

  return result;
}

enum rl_read_result
rl_read_world(SDL_IOStream* src, struct rl_world* world)
{
  struct rl_reader r = { 0 };

  enum rl_read_result result = read_entities(src, world, &r);
  if (result == RL_READ_OK) {
    result = read_levels(src, world, &r);
  }

  rl_free_reader(&r);

  return result;
}
