/**
 * @file event.h
 */
#ifndef GINC_ROGUELIKE_EVENT_H
#define GINC_ROGUELIKE_EVENT_H

#include <SDL3/SDL_rect.h>

#include "container/alist.h"

/**
 * The different types of events.
 */
enum rl_event_type
{
  RL_EVENT_ATTACK,
  RL_EVENT_DEATH,
  RL_EVENT_AWAKEN,
};

/**
 * An entity attacks.
 */
struct rl_event_attack
{
  /** Identifier of the attacking entity. */
  int attacker;
  /** Identifier of the defending entity. */
  int defender;
  /** Amount of damage done; -1 is a miss. */
  int damage;
};

/**
 * An entity dies.
 */
struct rl_event_death
{
  /** Identifier of the dying entity. */
  int entity;
  /** Identifier of the killing entity. */
  int killer;
};

/**
 * An entity awakens.
 */
struct rl_event_awaken
{
  /** Identifier of the now awake entity. */
  int entity;
};

/**
 * An event.
 */
struct rl_event
{
  enum rl_event_type type;

  union
  {
    struct rl_event_attack attack;
    struct rl_event_death death;
    struct rl_event_awaken awaken;
  } as;
};

/**
 * A growable array of events.
 */
alist_define_as(struct rl_event, rl_event);

#endif // GINC_ROGUELIKE_EVENT_H
