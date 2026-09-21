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
  RL_EVENT_ATTACK,       //< an actor attacks
  RL_EVENT_DEATH,        //< an actor dies
  RL_EVENT_AWAKEN,       //< an actor awakens
  RL_EVENT_PICKUP,       //< an actor picks up an item
  RL_EVENT_HEAL,         //< an actor is healed
  RL_EVENT_FEEDBACK,     //< a message for the player
  RL_EVENT_LEVEL_CHANGE, //< an actor moves to another level
  RL_EVENT_XP_GAIN,      //< an actor gains experience
  RL_EVENT_LEVEL_UP,     //< an actor gains one or more levels
  RL_EVENT_EQUIP,        //< an actor equips an item
  RL_EVENT_UNEQUIP,      //< an actor removes an equipped item
  RL_EVENT_DROP,         //< an actor drops an item
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

/** An actor equips or unequips an item. */
struct rl_event_equipment
{
  handle(rl_actor) actor;
  handle(rl_item) item;
};

/**
 * An actor drops an item.
 */
struct rl_event_drop
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
 * Feedback (i.e., a message) for the player.
 */
struct rl_event_feedback
{
  /** Message to show the player. */
  char const* message;
};

/**
 * An actor moves to another level.
 */
struct rl_event_level_change
{
  /** Handle of the actor. */
  handle(rl_actor) actor;
  /** Depth of the level left. */
  int from_depth;
  /** Depth of the level entered. */
  int to_depth;
};

/**
 * An actor gains experience.
 */
struct rl_event_xp_gain
{
  /** Handle of the actor. */
  handle(rl_actor) actor;
  /** Total XP awarded, including XP spent on level-ups. */
  int amount;
};

/**
 * An actor gains one or more levels from a single XP award.
 */
struct rl_event_level_up
{
  /** Handle of the actor. */
  handle(rl_actor) actor;
  /** Level before the award. */
  int from_level;
  /** Level after the award. */
  int to_level;
};

/**
 * An event.
 */
struct rl_event
{
  /** The kind of event. */
  enum rl_event_type type;

  /** The details of the event, matching type. */
  union
  {
    struct rl_event_attack attack;
    struct rl_event_death death;
    struct rl_event_awaken awaken;
    struct rl_event_pickup pickup;
    struct rl_event_heal heal;
    struct rl_event_feedback feedback;
    struct rl_event_level_change level_change;
    struct rl_event_xp_gain xp_gain;
    struct rl_event_level_up level_up;
    struct rl_event_equipment equipment;
    struct rl_event_drop drop;
  } as;
};

/**
 * A growable array of events.
 */
alist_define_as(struct rl_event, rl_event);

#endif // GINC_ROGUELIKE_EVENT_H
