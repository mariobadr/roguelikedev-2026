/**
 * @file save_format.h
 */
#ifndef GINC_ROGUELIKE_SAVE_FORMAT_H
#define GINC_ROGUELIKE_SAVE_FORMAT_H

#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_time.h>

struct rl_game;

/**
 * A stable identifier for one run, valid for as long as the run's save exists.
 */
struct rl_save_id
{
  Uint64 value;
};

/**
 * @return an identifier that refers to no run.
 */
#define rl_save_id_invalid() ((struct rl_save_id){ 0 })

/**
 * @return whether the identifier refers to a run.
 */
#define rl_save_id_is_valid(id) ((id).value != 0)

/**
 * @return whether two identifiers refer to the same run.
 */
#define rl_save_id_equal(a, b) ((a).value == (b).value)

/**
 * The result of a save operation.
 */
enum rl_save_result
{
  RL_SAVE_OK,           //< the operation succeeded
  RL_SAVE_MISSING,      //< no run has this identifier
  RL_SAVE_INCOMPATIBLE, //< the save was written for a different version
  RL_SAVE_CORRUPT,      //< the save is malformed
  RL_SAVE_UNAVAILABLE,  //< the run is finished and cannot be resumed
  RL_SAVE_ERROR,        //< filesystem or similar error; see SDL_GetError()
};

/**
 * The state of a saved game.
 */
enum rl_run_outcome
{
  RL_RUN_ACTIVE,  //< in progress, and resumable
  RL_RUN_DEAD,    //< the rogue died
  RL_RUN_VICTORY, //< the rogue won
};

/**
 * The condition of a save's contents.
 */
enum rl_save_condition
{
  RL_SAVE_CONDITION_OK,           //< readable by this build
  RL_SAVE_CONDITION_INCOMPATIBLE, //< written by a different format version
  RL_SAVE_CONDITION_CORRUPT,      //< malformed
};

/**
 * A summary of one run, enough to describe it without loading it.
 */
struct rl_save_info
{
  /** The run this summary describes. */
  struct rl_save_id id;
  /** Whether the rest of this summary could be read. */
  enum rl_save_condition condition;
  /** The state of the saved game. */
  enum rl_run_outcome outcome;
  /** The format version the save was written with. */
  Uint32 version;
  /** When the run's snapshot was last replaced. */
  SDL_Time saved_at;
  /** Turns completed by the run. */
  Uint64 turns;
  /** How deep the rogue had gone. */
  int depth;
  /** The rogue's hit points. */
  int hp;
  /** The rogue's maximum hit points. */
  int max_hp;
};

/**
 * Build the summary describing game, as of saved_at, for id and outcome.
 */
struct rl_save_info
rl_summarise_save(struct rl_game const* game,
                  struct rl_save_id id,
                  enum rl_run_outcome outcome,
                  SDL_Time saved_at);

/**
 * Write info's header followed by game's snapshot to dst.
 */
enum rl_save_result
rl_write_save(SDL_IOStream* dst,
              struct rl_save_info const* info,
              struct rl_game const* game);

/**
 * Read only src's summary, for describing a save without loading it.
 *
 * @param out id must be set by the caller beforehand.
 */
enum rl_save_result
rl_read_save_summary(SDL_IOStream* src, struct rl_save_info* out);

/**
 * Read src's summary and snapshot into game.
 *
 * @param out_info the id must be set by the caller beforehand.
 * @param game must be zero-initialized.
 */
enum rl_save_result
rl_read_save(SDL_IOStream* src,
             struct rl_save_info* out_info,
             struct rl_game* game);

/**
 * Rewrite an already-written save's outcome in place.
 *
 * @param io must be open for both reading and writing.
 */
enum rl_save_result
rl_patch_save_outcome(SDL_IOStream* io, enum rl_run_outcome outcome);

#endif // GINC_ROGUELIKE_SAVE_FORMAT_H
