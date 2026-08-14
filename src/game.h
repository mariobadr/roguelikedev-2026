/**
 * @file game.h
 */
#ifndef GINC_ROGUELIKE_GAME_H
#define GINC_ROGUELIKE_GAME_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "action.h"
#include "map.h"

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct inpt_state;
struct rl_resources;

/**
 * An entity in the game.
 */
struct rl_entity
{
  /** Location on map in tile coordinates. */
  SDL_Point position;
  /** Glyph representing the entity. */
  Uint8 glyph;
};

/**
 * The state of the roguelike game.
 */
struct rl_game
{
  /** A non-owning pointer to the game's resources */
  struct rl_resources const* resources;
  /** The player's next action */
  struct rl_action action;
  /** Time before the next action fires */
  float action_cooldown;
  /** The player entity */
  struct rl_entity rogue;
  /** One map (for now) */
  struct rl_world_map map;
};

/**
 * Initialize a new game.
 *
 * @param game The game to initialize.
 *
 * @return whether game was successfully initialized.
 */
bool
rl_init_game(struct rl_game* game, struct rl_resources const* resources);

/**
 * Respond to accumulated input for the current frame.
 *
 * @param game The game to update.
 * @param istate The input state captured for this frame.
 *
 * @return false if the application should quit, true otherwise.
 */
bool
rl_handle_input(struct rl_game* game, struct inpt_state const* istate);

/**
 * Advance the game by delta time.
 *
 * @param g The game to update.
 * @param dt Elapsed time since the last frame in seconds.
 */
void
rl_update_game(struct rl_game* game, float dt);

/**
 * Render the game's current frame.
 *
 * @param game     The game to render.
 * @param renderer The renderer used to draw everything.
 */
void
rl_render_game(struct rl_game* game, SDL_Renderer* renderer);

#endif // GINC_ROGUELIKE_GAME_H
