/**
 * @file entity.h
 */
#ifndef GINC_ROGUELIKE_ENTITY_H
#define GINC_ROGUELIKE_ENTITY_H

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// forward declarations
struct rl_tile_map;
struct rand_state;

/**
 * The different types of entities in the game.
 */
enum rl_entity_type
{
  RL_ENTITY_ROGUE,
  RL_ENTITY_RAT
};

/**
 * An entity in the game.
 */
struct rl_entity
{
  /** The type of entity. */
  enum rl_entity_type type;
  /** Dispalyed name. */
  char const* name;
  /** Location in tile coordinates. */
  SDL_Point position;
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
 * Create a new entity of the given type.
 */
struct rl_entity
rl_create_entity(enum rl_entity_type type);

/**
 * @return how much damage was done (or -1 for a miss).
 */
int
rl_attack_entity(struct rl_entity const* attacker,
                 struct rl_entity* defender,
                 struct rand_state* rng);

/**
 * @return whether entity is alive.
 */
static inline bool
rl_entity_is_alive(struct rl_entity const* entity)
{
  return entity->hp > 0;
}

#endif // GINC_ROGUELIKE_ENTITY_H