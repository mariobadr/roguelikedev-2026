/**
 * @file actor.h
 */
#ifndef GINC_ROGUELIKE_ACTOR_H
#define GINC_ROGUELIKE_ACTOR_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "game/actor_def.h"
#include "game/equipment.h"
#include "game/handles.h"
#include "game/item_def.h"

// forward declarations
struct rl_world;

/**
 * An actor in the game.
 */
struct rl_actor
{
  /** The type of actor. */
  enum rl_actor_type type;
  /** The actor's level, starting at 1. */
  int level;
  /** This actor's handle, so code holding a pointer can refer to it. */
  handle(rl_actor) handle;
  /** Displayed name. */
  char const* name;
  /** Location in tile coordinates. */
  SDL_Point pos;
  /** Whether the actor has noticed the player. */
  bool awake;
  /** The current number of hit points. */
  int hp;
  /** The actor's stats at its current level, before equipment bonuses. */
  struct rl_actor_stats stats;
  /** Equipped items. */
  struct rl_equipment equipment;
};

/**
 * Create a new actor. Its handle is invalid until the actor is added to a
 * world (see rl_create_actor).
 *
 * @param level must be at least 1.
 */
struct rl_actor
rl_make_actor(enum rl_actor_type type, int level);

/**
 * Heal an actor's hit points by up to amount.
 *
 * @return the amount hp actually increased by.
 */
int
rl_heal_actor(struct rl_actor* actor, int amount);

/**
 * @return the actor's stats including equipment bonuses.
 */
struct rl_actor_stats
rl_get_actor_stats(struct rl_world const* world, struct rl_actor const* actor);

/**
 * @return whether actor is alive.
 */
static inline bool
rl_actor_is_alive(struct rl_actor const* actor)
{
  return actor->hp > 0;
}

#endif // GINC_ROGUELIKE_ACTOR_H
