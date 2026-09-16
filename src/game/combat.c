#include "combat.h"

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

static void
enqueue_attack_event(struct rl_actor const* attacker,
                     struct rl_actor const* defender,
                     int damage,
                     alist(rl_event)* events)
{
  struct rl_event event = { 0 };
  event.type = RL_EVENT_ATTACK;
  event.as.attack.attacker = attacker->handle;
  event.as.attack.defender = defender->handle;
  event.as.attack.damage = damage;
  *alist_push(events) = event;
}

static void
enqueue_death_event(struct rl_actor const* actor,
                    struct rl_actor const* killer,
                    alist(rl_event)* events)
{
  struct rl_event event = { 0 };
  event.type = RL_EVENT_DEATH;
  event.as.death.actor = actor->handle;
  event.as.death.killer = killer->handle;
  *alist_push(events) = event;
}

int
rl_resolve_attack(struct rl_actor const* attacker,
                  struct rl_actor* defender,
                  int power,
                  alist(rl_event)* events,
                  struct rand_state* rng)
{
  int damage;
  if (rand_next_up_to(rng, 100) < MISS_CHANCE) {
    damage = -1;
  } else {
    // integer division truncates, but we avoid floating point (yay!)
    int const base = (int)rand_next_between(rng, power * 8 / 10, power * 12 / 10);
    // our random base damage is then mitigated by armor
    damage = base - (base * defender->armor / (defender->armor + ARMOR_SCALING));

    // don't let HP dip below 0
    defender->hp = SDL_max(0, defender->hp - damage);
  }

  enqueue_attack_event(attacker, defender, damage, events);

  if (defender->hp <= 0) {
    enqueue_death_event(defender, attacker, events);
  }

  return damage;
}

bool
rl_attack_melee(struct rl_world* world,
                handle(rl_actor) attacker_handle,
                handle(rl_actor) defender_handle,
                alist(rl_event)* events,
                struct rand_state* rng)
{
  if (handle_equal(attacker_handle, defender_handle)) {
    // can't attack yourself (?)
    return false;
  }

  struct rl_actor* attacker = get_living_actor(world, attacker_handle);
  if (attacker == NULL) {
    // attacker_handle is not valid
    return false;
  }

  struct rl_actor* defender = get_living_actor(world, defender_handle);
  if (defender == NULL) {
    // defender_handle is not valid
    return false;
  }

  if (!are_adjacent(attacker->pos, defender->pos)) {
    // only allow melee attacks
    return false;
  }

  // from the good old WoW days
  int const ap = 2 * attacker->strength;
  rl_resolve_attack(attacker, defender, ap, events, rng);

  return true;
}
