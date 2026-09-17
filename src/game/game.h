/**
 * @file game.h
 */
#ifndef GINC_ROGUELIKE_GAME_H
#define GINC_ROGUELIKE_GAME_H

#include <SDL3/SDL_stdinc.h>

#include "container/alist.h"
#include "container/grid.h"
#include "core/rand.h"

#include "command.h"
#include "event.h"
#include "fov.h"
#include "world.h"

/**
 * A game that represents one run of the roguelike.
 */
struct rl_game
{
  /** How many turns have been completed. */
  Uint64 turns;
  /** The random number generator */
  struct rand_state rng;
  /** The world, including the levels visited so far. */
  struct rl_world world;
  /** A map of distances to reach the player. */
  grid(int) distances;
  /** Player's field-of-view */
  struct rl_fov fov;
};

/**
 * Generate a new, fully playable game into out.
 *
 * @param out must be zero-initialized.
 */
bool
rl_new_game(struct rl_game* out, int width, int height, Uint64 seed);

/**
 * Free game's owned resources and zero it.
 */
void
rl_free_game(struct rl_game* game);

/**
 * Applies a command and, if it consumes a turn, advances the rest of the game.
 *
 * Events triggered by the update are appended to the events list.
 *
 * @return whether applying the command consumes a turn.
 */
bool
rl_update_game(struct rl_game* game,
               struct rl_command const* cmd,
               alist(rl_event)* events);

/**
 * Note: expects the current level and rogue to be populated; runtime buffers
 * must be empty.
 */
bool
rl_prepare_game(struct rl_game* game);

#endif // GINC_ROGUELIKE_GAME_H
