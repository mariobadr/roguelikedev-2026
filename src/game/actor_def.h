/**
 * @file actor_def.h
 */
#ifndef GINC_ROGUELIKE_ACTOR_DEF_H
#define GINC_ROGUELIKE_ACTOR_DEF_H

/**
 * The different types of actors in the game.
 */
enum rl_actor_type
{
  RL_ACTOR_ROGUE, //< the player
  RL_ACTOR_RAT    //< a dangerous mouse
};

/** Immutable data that defines an actor. */
struct rl_actor_def
{
  char const* name;
  int max_hp;
  int strength;
  int armor;
};

/**
 * @return the actor definition that corresponds to type.
 */
struct rl_actor_def const*
rl_get_actor_def(enum rl_actor_type type);

#endif // GINC_ROGUELIKE_ACTOR_DEF_H
