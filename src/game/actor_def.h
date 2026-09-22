/**
 * @file actor_def.h
 */
#ifndef GINC_ROGUELIKE_ACTOR_DEF_H
#define GINC_ROGUELIKE_ACTOR_DEF_H

#include "game/loot.h"

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
  RL_ACTOR_ROGUE,  //< the player
  RL_ACTOR_RAT,    //< a dangerous mouse
  RL_ACTOR_GOBLIN, //< an ugly goblin
  RL_ACTOR_TROLL,  //< a huge troll
  // bosses
  RL_ACTOR_RAT_KING,      //< the king of rats
  RL_ACTOR_GOBLIN_CHIEF,  //< the leader of the goblins
  RL_ACTOR_TROLL_WARLORD, //< a troll clad for war
  RL_ACTOR_DRAGON,        //< an ancient dragon
};

/** Combat stats. */
struct rl_actor_stats
{
  /** The maximum number of hit points. */
  int max_hp;
  /** Impacts the amount of damage done. */
  int strength;
  /** Impacts the chance of a critical hit. */
  int agility;
  /** Impacts the amount of damage mitigated. */
  int armor;
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
  /** The stats at level 1. */
  struct rl_actor_stats base;
  /** The stats gained per level. */
  struct rl_actor_stats per_level;
  /** What the actor may drop when it dies. */
  struct rl_loot_table loot;
};

/**
 * @return the actor definition that corresponds to type.
 */
struct rl_actor_def const*
rl_get_actor_def(enum rl_actor_type type);

#endif // GINC_ROGUELIKE_ACTOR_DEF_H
