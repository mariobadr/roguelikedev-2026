/**
 * @file actor.h
 */
#ifndef GINC_ROGUELIKE_ACTOR_H
#define GINC_ROGUELIKE_ACTOR_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "container/alist.h"

// forward declarations
struct rand_state;

/**
 * The different types of actors in the game.
 */
enum rl_actor_type
{
  RL_ACTOR_ROGUE, //< the player
  RL_ACTOR_RAT    //< a dangerous mouse
};

/**
 * An actor in the game.
 */
struct rl_actor
{
  /** The type of actor. */
  enum rl_actor_type type;
  /** A unique identifier. */
  int id;
  /** Dispalyed name. */
  char const* name;
  /** Location in tile coordinates. */
  SDL_Point pos;
  /** Whether the actor is "active". */
  bool awake;
  /** The current level. */
  int level;
  /** The current number of hit points. */
  int hp;
  /** The maximum number of hit points. */
  int max_hp;
  /** Impacts the amount of damage done. */
  int strength;
  /** Impacts the amount of damage mitigated. */
  int armor;
};

/**
 * A growable array of actors.
 */
alist_define_as(struct rl_actor, rl_actor);

/**
 * Create a new actor.
 */
struct rl_actor
rl_create_actor(enum rl_actor_type type, int id);

/**
 * Heal an actor's hit points by up to amount.
 * 
 * @return the amount hp actually increased by.
 */
int
rl_heal_actor(struct rl_actor *actor, int amount);

/**
 * @return whether actor is alive.
 */
static inline bool
rl_actor_is_alive(struct rl_actor const* actor)
{
  return actor->hp > 0;
}

#endif // GINC_ROGUELIKE_ACTOR_H