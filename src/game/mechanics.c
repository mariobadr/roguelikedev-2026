#include "mechanics.h"

#include "procgen/rand.h"

#include "actor.h"
#include "combat.h"
#include "fov.h"
#include "targeting.h"
#include "world.h"

static bool
are_adjacent(SDL_Point a, SDL_Point b)
{
  int const dx = SDL_abs(a.x - b.x);
  int const dy = SDL_abs(a.y - b.y);

  return dx + dy == 1;
}

static struct rl_actor*
get_living_actor(struct rl_world* world, handle(rl_actor) actor_handle)
{
  struct rl_actor* actor = rl_borrow_mut_actor(world, actor_handle);
  if (actor == NULL) {
    return NULL;
  }

  if (!rl_actor_is_alive(actor)) {
    return NULL;
  }

  return actor;
}

static bool
can_move(struct rl_world const* world, SDL_Point dst)
{
  struct rl_level const* level = rl_get_current_level(world);

  if (!grid_contains(&level->map, dst.x, dst.y)) {
    return false;
  }

  if (!rl_is_walkable(*grid_at(&level->map, dst.x, dst.y))) {
    return false;
  }

  if (handle_is_nonnull(rl_find_actor(world, level, dst))) {
    // note: rl_find_actor already ignores dead actors.
    return false;
  }

  return true;
}

static bool
use_item_heal(struct rl_actor* actor,
              int power,
              alist(rl_event)* events,
              struct rand_state* rng)
{
  if (actor->hp >= actor->max_hp) {
    struct rl_event event = { 0 };
    event.type = RL_EVENT_FEEDBACK;
    event.as.feedback.message = "You are already at full health.";
    *alist_push(events) = event;
    return false;
  }

  // TODO: make this more sophisticated?
  int const amount =
    (int)rand_next_between(rng, power * 8 / 10, power * 12 / 10);
  // apply the effect
  int const effective = rl_heal_actor(actor, amount);

  // communicate the event
  struct rl_event event = { 0 };
  event.type = RL_EVENT_HEAL;
  event.as.heal.actor = actor->handle;
  event.as.heal.total = amount;
  event.as.heal.effective = effective;
  *alist_push(events) = event;

  return true;
}

static bool
use_item_damage_area(struct rl_world* world,
                     struct rl_actor* actor,
                     struct rl_item_def const* idef,
                     SDL_Point centre,
                     alist(rl_event)* events,
                     struct rand_state* rng)
{
  struct rl_level const* level = rl_get_current_level(world);
  for (size_t i = 0; i < alist_len(&level->actors); i++) {
    struct rl_actor* defender =
      rl_borrow_mut_actor(world, *alist_at(&level->actors, i));
    if (defender == NULL || !rl_actor_is_alive(defender)) {
      continue;
    }

    if (!rl_item_affects_tile(idef, world, centre, defender->pos)) {
      continue;
    }

    rl_resolve_attack(actor, defender, idef->power, events, rng);
  }

  return true;
}

static bool
use_item_lightning(struct rl_actor* actor,
                   struct rl_world* world,
                   struct rl_fov const* fov,
                   int power,
                   alist(rl_event)* events,
                   struct rand_state* rng)
{
  handle(rl_actor) const target_handle =
    rl_find_nearest_visible_actor(world, fov, actor->handle);
  struct rl_actor* nearest = rl_borrow_mut_actor(world, target_handle);
  if (nearest == NULL) {
    struct rl_event event = { 0 };
    event.type = RL_EVENT_FEEDBACK;
    event.as.feedback.message = "There is no target in sight.";
    *alist_push(events) = event;
    return false;
  }

  rl_resolve_attack(actor, nearest, power, events, rng);

  return true;
}

bool
rl_move(struct rl_world* world, handle(rl_actor) actor_handle, SDL_Point dst)
{
  struct rl_actor* actor = get_living_actor(world, actor_handle);
  if (actor == NULL) {
    return false;
  }

  if (!are_adjacent(actor->pos, dst)) {
    return false;
  }

  if (!can_move(world, dst)) {
    return false;
  }

  actor->pos = dst;
  return true;
}

bool
rl_pick_up_item(struct rl_world* world,
                handle(rl_actor) actor_handle,
                SDL_Point dst,
                alist(rl_event)* events)
{
  struct rl_actor* actor = get_living_actor(world, actor_handle);
  if (actor == NULL) {
    return false;
  }

  if (actor->pos.x != dst.x || actor->pos.y != dst.y) {
    // actor is not at dst
    return false;
  }

  struct rl_level* level = rl_edit_current_level(world);
  handle(rl_item) const item_handle = rl_find_item(world, level, dst);
  struct rl_item* item = rl_borrow_mut_item(world, item_handle);
  if (item == NULL) {
    // no item at dst
    return false;
  }

  if (!rl_remove_item(level, item_handle)) {
    return false;
  }

  item->ltype = RL_ITEM_LOCATION_HELD;
  item->on.actor = actor->handle;

  struct rl_event event = { 0 };
  event.type = RL_EVENT_PICKUP;
  event.as.pickup.actor = actor->handle;
  event.as.pickup.item = item_handle;
  *alist_push(events) = event;

  return true;
}

bool
rl_use_item(struct rl_world* world,
            handle(rl_actor) actor_handle,
            handle(rl_item) item_handle,
            SDL_Point target,
            struct rl_fov const* fov,
            alist(rl_event)* events,
            struct rand_state* rng)
{
  struct rl_actor* actor = get_living_actor(world, actor_handle);
  if (actor == NULL) {
    return false;
  }

  struct rl_item const* item = rl_borrow_item(world, item_handle);
  if (item == NULL) {
    return false;
  }

  if (item->ltype != RL_ITEM_LOCATION_HELD ||
      !handle_equal(item->on.actor, actor->handle)) {
    return false;
  }

  struct rl_item_def const* idef = rl_get_item_def(item->itype);
  if (idef->target == RL_ITEM_TARGET_TILE &&
      !rl_is_valid_item_target(idef, world, fov, target)) {
    return false;
  }

  bool used = false;
  switch (idef->effect) {
    case RL_ITEM_EFFECT_HEAL:
      used = use_item_heal(actor, idef->power, events, rng);
      break;
    case RL_ITEM_EFFECT_DAMAGE_AREA:
      used = use_item_damage_area(world, actor, idef, target, events, rng);
      break;
    case RL_ITEM_EFFECT_DAMAGE_NEAREST:
      used = use_item_lightning(actor, world, fov, idef->power, events, rng);
      break;
    default:
      break;
  }

  if (used) {
    // the item is consumed
    pool_release(&world->items, item_handle);
  }

  return used;
}
