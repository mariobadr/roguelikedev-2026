#include "mechanics.h"

#include "procgen/rand.h"

#include "actor.h"
#include "world.h"

#define MISS_CHANCE 5
#define ARMOR_SCALING 20

static bool
are_adjacent(SDL_Point a, SDL_Point b)
{
  int const dx = SDL_abs(a.x - b.x);
  int const dy = SDL_abs(a.y - b.y);

  return dx + dy == 1;
}

static struct rl_actor*
get_living_actor(struct rl_world* world, int actor_id)
{
  struct rl_actor* actor = rl_edit_actor(world, actor_id);
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

  if (rl_find_actor(world, dst) != NULL) {
    // note: rl_find_actor already ignores dead actors.
    return false;
  }

  return true;
}

static int
attack_actor(struct rl_actor const* attacker,
             struct rl_actor* defender,
             struct rand_state* rng)
{
  if (rand_next_up_to(rng, 100) < MISS_CHANCE) {
    return -1;
  }

  // from the good old WoW days
  int const ap = 2 * attacker->strength;
  // integer division truncates, but we avoid floating point (yay!)
  int const base = (int)rand_next_between(rng, ap * 8 / 10, ap * 12 / 10);
  // our random base damage is then mitigated by armor
  int const damage =
    base - (base * defender->armor / (defender->armor + ARMOR_SCALING));

  // don't let HP dip below 0
  defender->hp = SDL_max(0, defender->hp - damage);

  return damage;
}

static bool
use_item_heal(struct rl_actor* actor,
              struct rl_item* item,
              int power,
              alist(rl_event) * events,
              struct rand_state* rng)
{
  if(actor->hp >= actor->max_hp) {
    return false;
  }

  // TODO: make this more sophisticated?
  int const amount = (int)rand_next_between(rng, power * 8 / 10, power * 12 / 10);
  // apply the effect
  int const effective = rl_heal_actor(actor, amount);

  // communicate the event
  struct rl_event event = { 0 };
  event.type = RL_EVENT_HEAL;
  event.as.heal.actor = actor->id;
  event.as.heal.total = amount;
  event.as.heal.effective = effective;
  *alist_push(events) = event;

  // mark the item as consumed
  item->ltype = RL_ITEM_LOCATION_NONE;

  return true;
}

bool
rl_move(struct rl_world* world, int actor_id, SDL_Point dst)
{
  struct rl_actor* actor = get_living_actor(world, actor_id);
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
rl_attack_melee(struct rl_world* world,
                int attacker_id,
                int defender_id,
                alist(rl_event) * events,
                struct rand_state* rng)
{
  if (attacker_id == defender_id) {
    // can't attack yourself (?)
    return false;
  }

  struct rl_actor* attacker = get_living_actor(world, attacker_id);
  if (attacker == NULL) {
    // attacker_id is not valid
    return false;
  }

  struct rl_actor* defender = get_living_actor(world, defender_id);
  if (defender == NULL) {
    // defender_id is not valid
    return false;
  }

  if (!are_adjacent(attacker->pos, defender->pos)) {
    // only allow melee attacks
    return false;
  }

  int const damage = attack_actor(attacker, defender, rng);
  struct rl_event event = { 0 };
  event.type = RL_EVENT_ATTACK;
  event.as.attack.attacker = attacker->id;
  event.as.attack.defender = defender->id;
  event.as.attack.damage = damage;
  *alist_push(events) = event;

  if (defender->hp <= 0) {
    event.type = RL_EVENT_DEATH;
    event.as.death.actor = defender->id;
    event.as.death.killer = attacker->id;
    *alist_push(events) = event;
  }

  return true;
}

bool
rl_pick_up_item(struct rl_world* world,
                int actor_id,
                SDL_Point dst,
                alist(rl_event) * events)
{
  struct rl_actor* actor = get_living_actor(world, actor_id);
  if (actor == NULL) {
    return false;
  }

  if (actor->pos.x != dst.x || actor->pos.y != dst.y) {
    // actor is not at dst
    return false;
  }

  struct rl_item* item = rl_find_item(world, dst);
  if (item == NULL) {
    // no item at dst
    return false;
  }

  item->ltype = RL_ITEM_LOCATION_HELD;
  item->on.actor = actor_id;

  struct rl_event event = { 0 };
  event.type = RL_EVENT_PICKUP;
  event.as.pickup.actor = actor_id;
  event.as.pickup.item = item->id;
  *alist_push(events) = event;

  return true;
}

bool
rl_use_item(struct rl_world* world,
            int actor_id,
            int item_id,
            alist(rl_event) * events,
            struct rand_state* rng)
{
  struct rl_actor* actor = get_living_actor(world, actor_id);
  if (actor == NULL) {
    return false;
  }

  struct rl_item* item = rl_edit_item(world, item_id);
  if (item == NULL) {
    return false;
  }

  if (item->ltype != RL_ITEM_LOCATION_HELD || item->on.actor != actor_id) {
    return false;
  }

  struct rl_item_def const* idef = rl_get_item_def(item->itype);
  switch (idef->effect) {
    case RL_ITEM_EFFECT_HEAL:
      return use_item_heal(actor, item, idef->power, events, rng);
    default:
      break;
  }

  return false;
}
