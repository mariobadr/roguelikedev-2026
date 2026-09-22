#include "mechanics.h"

#include <SDL3/SDL_assert.h>

#include "core/rand.h"

#include "actor.h"
#include "combat.h"
#include "equipment.h"
#include "experience.h"
#include "generate.h"
#include "item_def.h"
#include "loot.h"
#include "movement.h"
#include "targeting.h"
#include "world.h"

int
rl_gain_xp(struct rl_world* world, int amount, alist(rl_event)* events)
{
  if (amount <= 0) {
    return 0;
  }

  struct rl_actor* actor = rl_borrow_mut_actor(world, rl_get_rogue(world));

  struct rl_actor_def const* def = rl_get_actor_def(actor->type);
  int const awarded = amount;
  int const from_level = actor->level;
  int gained = 0;
  int needed = rl_xp_required(actor->level) - world->player.xp;
  while (amount >= needed) {
    amount -= needed;
    world->player.xp = 0;
    actor->level++;
    actor->stats.max_hp += def->per_level.max_hp;
    actor->stats.strength += def->per_level.strength;
    actor->stats.agility += def->per_level.agility;
    actor->stats.armor += def->per_level.armor;
    gained++;
    needed = rl_xp_required(actor->level);
  }

  if (gained > 0) {
    actor->hp = actor->stats.max_hp;
  }

  world->player.xp += amount;

  struct rl_event xp_event = { 0 };
  xp_event.type = RL_EVENT_XP_GAIN;
  xp_event.as.xp_gain.actor = actor->handle;
  xp_event.as.xp_gain.amount = awarded;
  *alist_push(events) = xp_event;

  if (gained > 0) {
    struct rl_event level_event = { 0 };
    level_event.type = RL_EVENT_LEVEL_UP;
    level_event.as.level_up.actor = actor->handle;
    level_event.as.level_up.from_level = from_level;
    level_event.as.level_up.to_level = actor->level;
    *alist_push(events) = level_event;
  }

  return gained;
}

static bool
can_move(struct rl_world const* world, SDL_Point dst)
{
  struct rl_level const* level = rl_get_current_level(world);

  if (!rl_can_walk(&level->map, dst)) {
    return false;
  }

  if (handle_is_nonnull(rl_find_actor(world, level, dst))) {
    // note: rl_find_actor already ignores dead actors.
    return false;
  }

  return true;
}

/**
 * Record an attack, along with the defender's death if it was the killing
 * blow.
 *
 * @param defender must have been alive before the attack.
 */
static void
enqueue_attack_events(struct rl_actor const* attacker,
                      struct rl_actor const* defender,
                      struct rl_attack attack,
                      alist(rl_event)* events)
{
  struct rl_event event = { 0 };
  event.type = RL_EVENT_ATTACK;
  event.as.attack.attacker = attacker->handle;
  event.as.attack.defender = defender->handle;
  event.as.attack.damage = attack.damage;
  event.as.attack.critical = attack.critical;
  *alist_push(events) = event;

  if (rl_actor_is_alive(defender)) {
    return;
  }

  struct rl_event death = { 0 };
  death.type = RL_EVENT_DEATH;
  death.as.death.actor = defender->handle;
  death.as.death.killer = attacker->handle;
  *alist_push(events) = death;
}

void
rl_attack_melee(struct rl_world const* world,
                struct rl_actor const* attacker,
                struct rl_actor* defender,
                alist(rl_event)* events,
                struct rand_state* rng)
{
  SDL_assert(rl_actor_is_alive(defender));

  // from the good old WoW days
  struct rl_actor_stats const stats = rl_get_actor_stats(world, attacker);
  int const ap = 2 * stats.strength + stats.agility;
  int const crit_chance = rl_get_crit_chance(stats.agility, attacker->level);
  int const armour = rl_get_actor_stats(world, defender).armor;

  struct rl_attack const attack =
    rl_resolve_attack(defender, ap, crit_chance, armour, rng);

  enqueue_attack_events(attacker, defender, attack, events);
}

void
rl_attack_magic(struct rl_actor const* attacker,
                struct rl_actor* defender,
                int power,
                alist(rl_event)* events,
                struct rand_state* rng)
{
  SDL_assert(rl_actor_is_alive(defender));

  struct rl_attack const attack = rl_resolve_attack(defender, power, 0, 0, rng);

  enqueue_attack_events(attacker, defender, attack, events);
}

static bool
use_item_heal(struct rl_actor* actor,
              int power,
              alist(rl_event)* events,
              struct rand_state* rng)
{
  if (actor->hp >= actor->stats.max_hp) {
    struct rl_event event = { 0 };
    event.type = RL_EVENT_FEEDBACK;
    event.as.feedback.message = "You are already at full health.";
    *alist_push(events) = event;
    return false;
  }

  // TODO: make this more sophisticated?
  struct rl_roll_range const range = rl_get_roll_range(power);
  int const amount = (int)rand_next_between(rng, range.min, range.max);
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
                     struct rl_item_consumable_def const* idef,
                     int power,
                     SDL_Point centre,
                     alist(rl_event)* events,
                     struct rand_state* rng)
{
  struct rl_level const* level = rl_get_current_level(world);
  for (size_t i = 0; i < alist_len(&level->actors); i++) {
    struct rl_actor* defender =
      rl_borrow_mut_actor(world, *alist_at(&level->actors, i));
    if (!rl_actor_is_alive(defender)) {
      continue;
    }

    if (!rl_item_affects_tile(idef, world, centre, defender->pos)) {
      continue;
    }

    rl_attack_magic(actor, defender, power, events, rng);
  }

  return true;
}

static bool
use_item_lightning(struct rl_actor* actor,
                   struct rl_world* world,
                   int power,
                   alist(rl_event)* events,
                   struct rand_state* rng)
{
  handle(rl_actor) const target_handle =
    rl_find_nearest_visible_actor(world, actor->handle);
  struct rl_actor* nearest = rl_borrow_mut_actor(world, target_handle);
  if (nearest == NULL) {
    struct rl_event event = { 0 };
    event.type = RL_EVENT_FEEDBACK;
    event.as.feedback.message = "There is no target in sight.";
    *alist_push(events) = event;
    return false;
  }

  rl_attack_magic(actor, nearest, power, events, rng);

  return true;
}

bool
rl_move(struct rl_world* world, handle(rl_actor) actor_handle, SDL_Point dst)
{
  struct rl_actor* actor = rl_borrow_mut_actor(world, actor_handle);
  if (!rl_are_adjacent(actor->pos, dst)) {
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
  struct rl_actor* actor = rl_borrow_mut_actor(world, actor_handle);
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

  rl_remove_item(level, item_handle);

  item->ltype = RL_ITEM_LOCATION_HELD;
  item->on.actor = actor->handle;

  enum rl_equipment_slot const slot = rl_get_equipment_slot(item->itype);
  if (slot == RL_EQUIPMENT_SLOT_NONE) {
    struct rl_event event = { 0 };
    event.type = RL_EVENT_PICKUP;
    event.as.pickup.actor = actor->handle;
    event.as.pickup.item = item_handle;
    *alist_push(events) = event;
    return true;
  }

  handle(rl_item) const previous_handle = rl_get_equipped_item(actor, slot);
  if (handle_is_nonnull(previous_handle)) {
    struct rl_item* previous = rl_borrow_mut_item(world, previous_handle);
    rl_unequip(actor, previous);
    previous->ltype = RL_ITEM_LOCATION_MAP;
    previous->on.map = dst;
    rl_add_item(level, previous_handle);
  }

  rl_equip(actor, item);

  struct rl_event event = { 0 };
  event.type = RL_EVENT_EQUIP;
  event.as.equipment.actor = actor->handle;
  event.as.equipment.item = item_handle;
  event.as.equipment.replaced = previous_handle;
  *alist_push(events) = event;

  return true;
}

void
rl_drop_loot(struct rl_world* world,
             handle(rl_actor) actor_handle,
             alist(rl_event)* events,
             struct rand_state* rng)
{
  struct rl_actor const* actor = rl_borrow_actor(world, actor_handle);

  struct rl_loot_table const* loot = &rl_get_actor_def(actor->type)->loot;

  enum rl_item_type type;
  if (!rl_roll_loot(loot, rng, &type)) {
    return;
  }

  struct rl_level* level = rl_edit_current_level(world);
  handle(rl_item) const item_handle = rl_add_item_to_level(
    world, level, type, actor->level + loot->level_bonus, actor->pos);

  struct rl_event* event = alist_push(events);
  *event = (struct rl_event){ 0 };
  event->type = RL_EVENT_DROP;
  event->as.drop.actor = actor_handle;
  event->as.drop.item = item_handle;
}

/**
 * Add the actor to the level at index, at the staircase it arrives by.
 */
static bool
enter_existing_level(struct rl_world* world,
                     int index,
                     bool going_down,
                     handle(rl_actor) actor_handle)
{
  struct rl_level* level = alist_at(&world->levels, index);
  if (!rl_add_actor(level, actor_handle)) {
    return false;
  }

  struct rl_actor* actor = rl_borrow_mut_actor(world, actor_handle);
  actor->pos = going_down ? level->stairs_up : level->stairs_down;
  return true;
}

bool
rl_can_take_stairs(struct rl_world const* world, struct rl_actor const* actor)
{
  if (!handle_equal(actor->handle, rl_get_rogue(world))) {
    return false;
  }

  struct rl_level const* level = rl_get_current_level(world);
  switch (*grid_at(&level->map, actor->pos.x, actor->pos.y)) {
    case RL_TILE_STAIRS_DOWN:
      return true;
    case RL_TILE_STAIRS_UP:
      return world->current_level > 0;
    default:
      return false;
  }
}

bool
rl_take_stairs(struct rl_world* world,
               handle(rl_actor) actor_handle,
               alist(rl_event)* events,
               struct rand_state* rng)
{
  struct rl_actor const* actor = rl_borrow_actor(world, actor_handle);
  if (!rl_can_take_stairs(world, actor)) {
    return false;
  }

  // the level pointer is invalid once a level is pushed, so keep what is needed
  struct rl_level const* level = rl_get_current_level(world);
  int const from = world->current_level;
  int const from_depth = level->depth;
  int const width = grid_width(&level->map);
  int const height = grid_height(&level->map);

  bool const going_down =
    *grid_at(&level->map, actor->pos.x, actor->pos.y) == RL_TILE_STAIRS_DOWN;
  int const to = going_down ? from + 1 : from - 1;

  if (to < (int)alist_len(&world->levels)) {
    if (!enter_existing_level(world, to, going_down, actor_handle)) {
      return false;
    }
  } else if (!rl_push_level(world, width, height, actor_handle, rng)) {
    return false;
  }

  // the actor is now on both levels; leave the old one
  rl_remove_actor(alist_at(&world->levels, from), actor_handle);
  world->current_level = to;

  struct rl_event event = { 0 };
  event.type = RL_EVENT_LEVEL_CHANGE;
  event.as.level_change.actor = actor_handle;
  event.as.level_change.from_depth = from_depth;
  event.as.level_change.to_depth = alist_at(&world->levels, to)->depth;
  *alist_push(events) = event;

  return true;
}

bool
rl_use_item(struct rl_world* world,
            handle(rl_actor) actor_handle,
            handle(rl_item) item_handle,
            SDL_Point target,
            alist(rl_event)* events,
            struct rand_state* rng)
{
  struct rl_actor* actor = rl_borrow_mut_actor(world, actor_handle);
  struct rl_item const* item = rl_borrow_item(world, item_handle);
  if (item == NULL) {
    return false;
  }

  if (item->ltype != RL_ITEM_LOCATION_HELD ||
      !handle_equal(item->on.actor, actor->handle)) {
    return false;
  }

  struct rl_item_consumable_def const* idef =
    rl_get_item_consumable_def(item->itype);
  if (idef == NULL) {
    return false;
  }

  if (idef->target == RL_ITEM_TARGET_TILE &&
      !rl_is_valid_item_target(idef, world, target)) {
    return false;
  }

  int const power = rl_get_item_power(item);
  bool used = false;
  switch (idef->effect) {
    case RL_ITEM_EFFECT_HEAL:
      used = use_item_heal(actor, power, events, rng);
      break;
    case RL_ITEM_EFFECT_DAMAGE_AREA:
      used =
        use_item_damage_area(world, actor, idef, power, target, events, rng);
      break;
    case RL_ITEM_EFFECT_DAMAGE_NEAREST:
      used = use_item_lightning(actor, world, power, events, rng);
      break;
  }

  if (used) {
    // the item is consumed
    pool_release(&world->items, item_handle);
  }

  return used;
}
