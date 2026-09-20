/**
 * @file game.h
 */
#ifndef GINC_ROGUELIKE_GAME_H
#define GINC_ROGUELIKE_GAME_H

#include <SDL3/SDL_stdinc.h>

#include "container/alist.h"
#include "core/rand.h"

#include "command.h"
#include "event.h"
#include "world.h"

/**
 * A game that represents one run of the roguelike.
 */
struct rl_game
{
  /** How many turns have been completed. */
  Uint64 turns;
  /** The random number generator. */
  struct rand_state rng;
  /** The game world. */
  struct rl_world world;
};

/**
 * Generate a new, fully playable game into out.
 *
 * @param out must be zero-initialised.
 *
 * @return whether generation succeeded.
 */
bool
rl_new_game(struct rl_game* out, int width, int height, Uint64 seed);

/**
 * Free game's owned resources and zero it.
 */
void
rl_free_game(struct rl_game* game);

/**
 * Apply a command and, if it consumes a turn, advances the rest of the game.
 *
 * Events triggered by the update are appended to the events list.
 *
 * @return whether applying the command consumes a turn.
 */
bool
rl_update_game(struct rl_game* game,
               struct rl_command const* cmd,
               alist(rl_event)* events);

#endif // GINC_ROGUELIKE_GAME_H
