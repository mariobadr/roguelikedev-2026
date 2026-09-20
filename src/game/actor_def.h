/**
 * @file actor_def.h
 */
#ifndef GINC_ROGUELIKE_ACTOR_DEF_H
#define GINC_ROGUELIKE_ACTOR_DEF_H

/** Actor categories. */
enum rl_actor_class
{
  RL_ACTOR_HUMANOID, //< a person
  RL_ACTOR_BEAST,    //< an animal
};

/**
 * Possible actors encountered in the game.
 */
enum rl_actor_type
{
  RL_ACTOR_ROGUE, //< the player
  RL_ACTOR_RAT    //< a dangerous mouse
};

/**
 * Immutable data that defines an actor.
 */
struct rl_actor_def
{
  /** The category of the actor. */
  enum rl_actor_class class;

  /** The display name. */
  char const* name;
  /** The maximum hit points at level 1. */
  int base_hp;
  /** The maximum hit points gained per level. */
  int hp_per_level;

  /** The strength at level 1. */
  int base_strength;
  /** The strength gained per level. */
  int strength_per_level;

  /** The armor at level 1. */
  int base_armor;
  /** The armor gained per level. */
  int armor_per_level;
};

/**
 * @return the actor definition that corresponds to type.
 */
struct rl_actor_def const*
rl_get_actor_def(enum rl_actor_type type);

#endif // GINC_ROGUELIKE_ACTOR_DEF_H
