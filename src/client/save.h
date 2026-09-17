/**
 * @file save.h
 */
#ifndef GINC_ROGUELIKE_SAVE_H
#define GINC_ROGUELIKE_SAVE_H

#include "container/alist.h"

#include "client/save_format.h"

// forward declarations
struct rl_game;

/**
 * A growable list of run summaries.
 */
alist_define_as(struct rl_save_info, rl_save_info);

/**
 * Summarise every saved run into out.
 * 
 * Sorted by most-recent-first.
 *
 * @return whether the runs were written to out successfully.
 */
bool
rl_list_saves(alist(rl_save_info)* out);

/**
 * This is a summary-only check; it does not guarantee the full snapshot
 * decodes successfully.
 * 
 * @return whether info describes a save that can be continued
 */
bool
rl_save_is_continuable(struct rl_save_info const* info);

/**
 * Begin a new run.
 *
 * @param out set to the run's identifier only on success.
 */
enum rl_save_result
rl_create_save(struct rl_game const* game, struct rl_save_id* out);

/**
 * Replace the run's snapshot with game (atomically).
 *
 * On failure, the previous snapshot is left intact.
 */
enum rl_save_result
rl_save_game(struct rl_save_id id, struct rl_game const* game);

/**
 * Restore the run's snapshot into game; delegates entirely to rl_read_save.
 *
 * @param game must be zero-initialized. Left untouched on any non-OK result.
 */
enum rl_save_result
rl_load_game(struct rl_save_id id, struct rl_game* game);

/**
 * End the run (so that it can no longer be resumed).
 */
enum rl_save_result
rl_finish_save(struct rl_save_id id, enum rl_run_outcome outcome);

/**
 * Delete the run.
 */
enum rl_save_result
rl_delete_save(struct rl_save_id id);

#endif // GINC_ROGUELIKE_SAVE_H
