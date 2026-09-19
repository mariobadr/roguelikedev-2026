/**
 * @file actor_def.h
 */
#ifndef GINC_ROGUELIKE_ACTOR_DEF_H
#define GINC_ROGUELIKE_ACTOR_DEF_H

/** Actor categories. */
enum rl_actor_class
{
  RL_ACTOR_HUMANOID,
  RL_ACTOR_BEAST,
};

/**
 * Possible actors encountered in the game.
 */
enum rl_actor_type
{
  RL_ACTOR_ROGUE, //< the player
  RL_ACTOR_RAT    //< a dangerous mouse
};

/** Immutable data that defines an actor. */
struct rl_actor_def
{
  enum rl_actor_class class;

  char const* name;
  int base_hp;
  int hp_per_level;

  int base_strength;
  int strength_per_level;

  int base_armor;
  int armor_per_level;
};

/**
 * @return the actor definition that corresponds to type.
 */
struct rl_actor_def const*
rl_get_actor_def(enum rl_actor_type type);

#endif // GINC_ROGUELIKE_ACTOR_DEF_H
