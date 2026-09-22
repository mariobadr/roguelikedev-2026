/**
 * @file combat.h
 */
#ifndef GINC_ROGUELIKE_COMBAT_H
#define GINC_ROGUELIKE_COMBAT_H

#include <SDL3/SDL_stdinc.h>

// forward declarations
struct rl_actor;
struct rand_state;

/** The inclusive bounds of a roll. */
struct rl_roll_range
{
  /** The lowest possible result. */
  int min;
  /** The highest possible result. */
  int max;
};

/**
 * What an attack did to its defender.
 */
struct rl_attack
{
  /** The damage dealt, or -1 on a miss. */
  int damage;
  /** Whether the hit was critical. */
  bool critical;
};

/**
 * @return the range a roll of power can land in.
 */
struct rl_roll_range
rl_get_roll_range(int power);

/**
 * @return the chance of a critical hit, in tenths of a percent.
 */
int
rl_get_crit_chance(int agility, int level);

/**
 * Roll an attack and subtract the damage from the defender's hit points.
 *
 * @param crit_chance is in tenths of a percent; an attack with 0 cannot crit.
 *
 * @return what the attack did.
 */
struct rl_attack
rl_resolve_attack(struct rl_actor* defender,
                  int power,
                  int crit_chance,
                  int armour,
                  struct rand_state* rng);

#endif // GINC_ROGUELIKE_COMBAT_H
