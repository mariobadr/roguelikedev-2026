/**
 * @file actor.h
 */
#ifndef GINC_ROGUELIKE_ACTOR_H
#define GINC_ROGUELIKE_ACTOR_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "game/actor_def.h"
#include "game/handles.h"

/**
 * An actor in the game.
 */
struct rl_actor
{
  /** The type of actor. */
  enum rl_actor_type type;
  /** This actor's handle, so code holding a pointer can refer to it. */
  handle(rl_actor) handle;
  /** Dispalyed name. */
  char const* name;
  /** Location in tile coordinates. */
  SDL_Point pos;
  /** Whether the actor is "active". */
  bool awake;
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
 * Create a new actor. Its handle is invalid until the actor is added to a
 * world (see rl_create_actor).
 */
struct rl_actor
rl_make_actor(enum rl_actor_type type);

/**
 * Heal an actor's hit points by up to amount.
 *
 * @return the amount hp actually increased by.
 */
int
rl_heal_actor(struct rl_actor* actor, int amount);

/**
 * @return whether actor is alive.
 */
static inline bool
rl_actor_is_alive(struct rl_actor const* actor)
{
  return actor->hp > 0;
}

#endif // GINC_ROGUELIKE_ACTOR_H
