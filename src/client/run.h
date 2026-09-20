/**
 * @file run.h
 */
#ifndef GINC_ROGUELIKE_RUN_H
#define GINC_ROGUELIKE_RUN_H

#include <SDL3/SDL_stdinc.h>

#include "save/format.h"

#include "game/game.h"

/**
 * A playable game and its save identity.
 */
struct rl_run
{
  /** The game being played. */
  struct rl_game game;
  /** Identifies where the run is saved. */
  struct rl_save_id save_id;
};

/**
 * The outcomes of starting or resuming a run.
 */
enum rl_run_result_type
{
  RL_RUN_RESULT_OK,         //< success
  RL_RUN_RESULT_GAME_ERROR, //< constructing the game failed
  RL_RUN_RESULT_SAVE_ERROR, //< loading a save failed
};

/**
 * The result of starting or resuming a run.
 */
struct rl_run_result
{
  /** The outcome. */
  enum rl_run_result_type type;
  /** Valid when type is RL_RUN_RESULT_SAVE_ERROR. */
  enum rl_save_result save_error;
};

/**
 * Start a new run with a map of width by height tiles, generated from seed.
 *
 * @return the result. run is unchanged unless it succeeded.
 */
struct rl_run_result
rl_start_run(struct rl_run* run, int width, int height, Uint64 seed);

/**
 * Free the run.
 */
void
rl_free_run(struct rl_run* run);

/**
 * Resume the run saved as id.
 *
 * @return the result. run is unchanged unless it succeeded.
 */
struct rl_run_result
rl_resume_run(struct rl_run* run, struct rl_save_id id);

/**
 * Save the run.
 *
 * @return the result of saving.
 */
enum rl_save_result
rl_save_run(struct rl_run const* run);

#endif // GINC_ROGUELIKE_RUN_H
