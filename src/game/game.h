/**
 * @file game.h
 */
#ifndef GINC_ROGUELIKE_GAME_H
#define GINC_ROGUELIKE_GAME_H

#include <SDL3/SDL_stdinc.h>

#include "container/alist.h"
#include "container/grid.h"

#include "actor.h"
#include "command.h"
#include "event.h"
#include "fov.h"
#include "procgen/rand.h"
#include "world.h"

/**
 * The state of a game.
 *
 * A game represents one run of the roguelike game.
 */
struct rl_game
{
  /** The random number generator */
  struct rand_state rng;
  /** The world, including the levels visited so far. */
  struct rl_world world;
  /** A map of distances to reach the player. */
  grid(int) distances;
  /** Player's field-of-view */
  struct rl_fov fov;
};

bool
rl_alloc_game(struct rl_game* game);

void
rl_free_game(struct rl_game* game);

bool
rl_new_game(struct rl_game* game, int width, int height, Uint64 seed);

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

#endif // GINC_ROGUELIKE_GAME_H
