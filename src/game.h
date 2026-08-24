/**
 * @file game.h
 */
#ifndef GINC_ROGUELIKE_GAME_H
#define GINC_ROGUELIKE_GAME_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "action.h"
#include "command.h"
#include "entity.h"
#include "fov.h"
#include "rand.h"
#include "world.h"

#define MAP_WIDTH 80
#define MAP_HEIGHT 40

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct inpt_state;
struct rl_resources;

/**
 * The state of the roguelike game.
 */
struct rl_game
{
  /** A non-owning pointer to the game's resources */
  struct rl_resources const* resources;
  /** The random number generator */
  struct rand_state rng;
  /** The player's next action */
  enum rl_action action;
  /** Time before the next action fires */
  float action_cooldown;
  /** One map (for now) */
  struct rl_world world;
  /** Player's field-of-view */
  struct rl_fov fov;
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
 * Free the resources owned by game.
 */
void
rl_free_game(struct rl_game* game);

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
