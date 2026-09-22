#include "combat.h"

#include "core/rand.h"

#include "actor.h"
#include "world.h"

#define MISS_CHANCE 5
#define ARMOR_SCALING 20
// in tenths of a percent
#define BASE_CRIT_CHANCE 50
#define CRIT_MULTIPLIER 2

static bool
are_adjacent(SDL_Point a, SDL_Point b)
{
  int const dx = SDL_abs(a.x - b.x);
  int const dy = SDL_abs(a.y - b.y);

  return dx + dy == 1;
}

static void
enqueue_attack_event(struct rl_actor const* attacker,
                     struct rl_actor const* defender,
                     int damage,
                     bool critical,
                     alist(rl_event)* events)
{
  struct rl_event event = { 0 };
  event.type = RL_EVENT_ATTACK;
  event.as.attack.attacker = attacker->handle;
  event.as.attack.defender = defender->handle;
  event.as.attack.damage = damage;
  event.as.attack.critical = critical;
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

struct rl_roll_range
rl_get_roll_range(int power)
{
  // integer division truncates, but we avoid floating point (yay!)
  return (struct rl_roll_range){
    .min = power * 8 / 10,
    .max = power * 12 / 10,
  };
}

static int
get_crit_chance(struct rl_world const* world, struct rl_actor const* actor)
{
  // like WoW, each point of agility is worth less as the actor levels up
  int const agility = rl_get_actor_stats(world, actor).agility;
  return BASE_CRIT_CHANCE + agility * 100 / (actor->level + 9);
}

static int
resolve_attack(struct rl_actor const* attacker,
               struct rl_actor* defender,
               int power,
               int crit_chance,
               int armour,
               alist(rl_event)* events,
               struct rand_state* rng)
{
  int damage;
  bool critical = false;
  if (rand_next_up_to(rng, 100) < MISS_CHANCE) {
    damage = -1;
  } else {
    struct rl_roll_range const range = rl_get_roll_range(power);
    int base = (int)rand_next_between(rng, range.min, range.max);
    if (rand_next_up_to(rng, 1000) < (Uint64)crit_chance) {
      critical = true;
      base *= CRIT_MULTIPLIER;
    }
    // our random base damage is then mitigated by armour
    damage = base - (base * armour / (armour + ARMOR_SCALING));

    // don't let HP dip below 0
    defender->hp = SDL_max(0, defender->hp - damage);
  }

  enqueue_attack_event(attacker, defender, damage, critical, events);

  if (defender->hp <= 0) {
    enqueue_death_event(defender, attacker, events);
  }

  return damage;
}

int
rl_attack_magic(struct rl_actor const* attacker,
                struct rl_actor* defender,
                int power,
                alist(rl_event)* events,
                struct rand_state* rng)
{
  // magic ignores armour and cannot crit
  return resolve_attack(attacker, defender, power, 0, 0, events, rng);
}

bool
rl_attack_melee(struct rl_world* world,
                handle(rl_actor) attacker_handle,
                handle(rl_actor) defender_handle,
                alist(rl_event)* events,
                struct rand_state* rng)
{
  struct rl_actor* attacker = rl_borrow_mut_actor(world, attacker_handle);
  struct rl_actor* defender = rl_borrow_mut_actor(world, defender_handle);
  if (defender == NULL || !rl_actor_is_alive(defender)) {
    // defender_handle is not valid
    return false;
  }

  if (!are_adjacent(attacker->pos, defender->pos)) {
    // only allow melee attacks
    return false;
  }

  // from the good old WoW days
  struct rl_actor_stats const stats = rl_get_actor_stats(world, attacker);
  int const ap = 2 * stats.strength + stats.agility;
  int const crit_chance = get_crit_chance(world, attacker);
  int const armour = rl_get_actor_stats(world, defender).armor;
  resolve_attack(attacker, defender, ap, crit_chance, armour, events, rng);

  return true;
}
