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
  /** Dispalyed name. */
  char const* name;
  /** Glyph representing the entity. */
  Uint8 glyph;
  /** The glyph's colour. */
  SDL_FColor colour;
  /** Location in tile coordinates. */
  SDL_Point position;
};

/**
 * Create a new entity of the given type.
 */
struct rl_entity
rl_create_entity(enum rl_entity_type type);

/**
 * Move entity along the move_vector if it is possible to on map.
 *
 * @param entity      the entity to update
 * @param map         the map the entity is moving on
 * @param move_vector the direction the entity is trying to move in
 *
 * @return whether the entity was updated (i.e., moved)
 */
bool
rl_move_entity(struct rl_entity* entity,
               struct rl_tile_map const* map,
               SDL_Point move_vector);


#endif // GINC_ROGUELIKE_ENTITY_H