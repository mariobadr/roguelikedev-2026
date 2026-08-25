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
};

/**
 * Create a new entity of the given type.
 */
struct rl_entity
rl_create_entity(enum rl_entity_type type);

#endif // GINC_ROGUELIKE_ENTITY_H