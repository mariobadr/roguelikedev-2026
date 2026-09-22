/**
 * @file generate.h
 */
#ifndef GINC_ROGUELIKE_GENERATE_H
#define GINC_ROGUELIKE_GENERATE_H

#include <SDL3/SDL_stdinc.h>

#include "game/handles.h"

/** The depth of the deepest level, which has no stairs down. */
#define RL_FINAL_DEPTH 10

// forward declarations
struct rand_state;
struct rl_world;

/**
 * Generate a level below the deepest one.
 *
 * The world's current level is unchanged, and the arriving actor is not removed
 * from the level it was on.
 *
 * @return whether the level was added; on failure the world is unchanged.
 */
bool
rl_push_level(struct rl_world* world,
              int width,
              int height,
              handle(rl_actor) arriving,
              struct rand_state* rng);

#endif // GINC_ROGUELIKE_GENERATE_H
