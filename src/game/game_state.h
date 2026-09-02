/**
 * @file game_state.h
 */
#ifndef GINC_ROGUELIKE_GAME_STATE_H
#define GINC_ROGUELIKE_GAME_STATE_H

#include <SDL3/SDL_stdinc.h>

#include "container/alist.h"

#include "action.h"
#include "entity.h"
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
  int map_width;
  int map_height;
  /** The random number generator */
  struct rand_state rng;
  /** Time before the next action fires */
  float action_cooldown;
  /** One map (for now) */
  struct rl_world world;
  /** Player's field-of-view */
  struct rl_fov fov;
  /** Events triggered during updates. */
  alist(rl_event) events;
};

bool
rl_init_game_state(struct rl_game_state* game_state,
                   int map_width,
                   int map_height);

void
rl_free_game_state(struct rl_game_state* game_state);

void
rl_update_game_state(struct rl_game_state* game_state,
                     enum rl_action action,
                     float dt);

#endif // GINC_ROGUELIKE_GAME_STATE_H
