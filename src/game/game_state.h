/**
 * @file game_state.h
 */
#ifndef GINC_ROGUELIKE_GAME_STATE_H
#define GINC_ROGUELIKE_GAME_STATE_H

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
 * A game state represents one run of the roguelike game.
 */
struct rl_game_state
{
  /** The random number generator */
  struct rand_state rng;
  /** One map (for now) */
  struct rl_world world;
  /** A map of distances to reach the player. */
  grid(int) distances;
  /** Player's field-of-view */
  struct rl_fov fov;
  /** Events triggered during updates. */
  alist(rl_event) events;
};

bool
rl_alloc_game_state(struct rl_game_state* game_state,
                    int map_width,
                    int map_height);

void
rl_free_game_state(struct rl_game_state* game_state);

/**
 * @return whether applying the command consumes a turn.
 */
bool
rl_update_game_state(struct rl_game_state* game_state,
                     struct rl_command const* cmd);

#endif // GINC_ROGUELIKE_GAME_STATE_H
