/**
 * @file game.h
 */
#ifndef GINC_ROGUELIKE_GAME_H
#define GINC_ROGUELIKE_GAME_H

#include <SDL3/SDL_stdinc.h>

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct inpt_state;

struct rl_game
{
  int placeholder;
};

/**
 * Initialize a new game.
 *
 * @param game The game to initialize.
 *
 * @return whether game was successfully initialized.
 */
bool
rl_init_game(struct rl_game* game);

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
 * @param game The game to render.
 */
void
rl_render_game(struct rl_game* game, SDL_Renderer* renderer);

#endif // GINC_ROGUELIKE_GAME_H
