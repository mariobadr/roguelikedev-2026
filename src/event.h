/**
 * @file event.h
 */
#ifndef GINC_ROGUELIKE_EVENT_H
#define GINC_ROGUELIKE_EVENT_H

#include <SDL3/SDL_rect.h>

#include "alist.h"

/**
 * The different types of events.
 */
enum rl_event_type
{
  RL_EVENT_MOVE,
  RL_EVENT_ATTACK,
  RL_EVENT_DEATH,
};

/**
 * Entity movement.
 */
struct rl_event_move
{
  /** Identifier of the moving entity. */
  int entity;
  /** Entity's current position. */
  SDL_Point position;
};

/**
 * Entity attacks.
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

struct rl_event_death
{
  /** Identifier of the dying entity. */
  int entity;
  /** Identifier of the killing entity. */
  int killer;
};

/**
 * An event.
 */
struct rl_event
{
  enum rl_event_type type;

  union
  {
    struct rl_event_move move;
    struct rl_event_attack attack;
    struct rl_event_death death;
  } as;
};

/**
 * A growable array of events.
 */
alist_define_as(struct rl_event, rl_event);

#endif // GINC_ROGUELIKE_EVENT_H