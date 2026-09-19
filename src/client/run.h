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
  struct rl_game game;
  struct rl_save_id save_id;
};

enum rl_run_result_type
{
  RL_RUN_RESULT_OK,         //< success
  RL_RUN_RESULT_GAME_ERROR, //< constructing the game failed
  RL_RUN_RESULT_SAVE_ERROR, //< loading a save failed
};

struct rl_run_result
{
  enum rl_run_result_type type;
  /** Valid when type is RL_RUN_RESULT_SAVE_ERROR. */
  enum rl_save_result save_error;
};

struct rl_run_result
rl_start_run(struct rl_run* run, int width, int height, Uint64 seed);

void
rl_free_run(struct rl_run* run);

struct rl_run_result
rl_resume_run(struct rl_run* run, struct rl_save_id id);

enum rl_save_result
rl_save_run(struct rl_run const* run);

#endif // GINC_ROGUELIKE_RUN_H
