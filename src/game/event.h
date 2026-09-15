/**
 * @file event.h
 */
#ifndef GINC_ROGUELIKE_EVENT_H
#define GINC_ROGUELIKE_EVENT_H

#include <SDL3/SDL_rect.h>

#include "container/alist.h"

#include "game/handles.h"

/**
 * The different types of events.
 */
enum rl_event_type
{
  RL_EVENT_ATTACK,
  RL_EVENT_DEATH,
  RL_EVENT_AWAKEN,
  RL_EVENT_PICKUP,
  RL_EVENT_HEAL,
  RL_EVENT_FEEDBACK,
};

/**
 * An actor attacks.
 */
struct rl_event_attack
{
  /** Handle of the attacking actor. */
  handle(rl_actor) attacker;
  /** Handle of the defending actor. */
  handle(rl_actor) defender;
  /** Amount of damage done; -1 is a miss. */
  int damage;
};

/**
 * An actor dies.
 */
struct rl_event_death
{
  /** Handle of the dying actor. */
  handle(rl_actor) actor;
  /** Handle of the killing actor. */
  handle(rl_actor) killer;
};

/**
 * An actor awakens.
 */
struct rl_event_awaken
{
  /** Handle of the now awake actor. */
  handle(rl_actor) actor;
};

/**
 * An actor picks up an item.
 */
struct rl_event_pickup
{
  /** Handle of the actor. */
  handle(rl_actor) actor;
  /** Handle of the item. */
  handle(rl_item) item;
};

/**
 * An actor is healed.
 */
struct rl_event_heal
{
  /** Handle of the actor being healed. */
  handle(rl_actor) actor;
  /** Maximum amount of healing. */
  int total;
  /** Actual amount of healing. */
  int effective;
};

/**
 * Feedback (i.e., a message) for the player..
 */
struct rl_event_feedback
{
  /** Message to show the player. */
  char const* message;
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
    struct rl_event_pickup pickup;
    struct rl_event_heal heal;
    struct rl_event_feedback feedback;
  } as;
};

/**
 * A growable array of events.
 */
alist_define_as(struct rl_event, rl_event);

#endif // GINC_ROGUELIKE_EVENT_H
