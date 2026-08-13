/**
 * @file rand.h
 */
#ifndef GINC_ROGUELIKE_RAND_H
#define GINC_ROGUELIKE_RAND_H

#include <SDL3/SDL_stdinc.h>

/**
 * State of the random number generator.
 *
 * From the original authors: This is xoshiro256** 1.0, one of our all-purpose,
 * rock-solid generators. It has excellent (sub-ns) speed, a state (256 bits)
 * that is large enough for any parallel application, and it passes all tests we
 * are aware of.
 */
struct rand_state
{
  Uint64 s[4];
};

/**
 * Seed a random number generator.
 *
 * From the original authors: The state must be seeded so that it is not
 * everywhere zero. If you have a 64-bit seed, we suggest to seed a splitmix64
 * generator and use its output to fill s.
 *
 * @param state State of the random number generator.
 * @param seed  A seed value.
 */
void
rand_seed(struct rand_state* state, Uint64 seed);

/**
 * Generate the next random number.
 *
 * @param state State of the random number generator.
 *
 * @return the random number
 */
Uint64
rand_next(struct rand_state* state);

/**
 * Advance state by 2^128 steps.
 *
 * From the original authors: This is the jump function for the generator. It is
 * equivalent to 2^128 calls to next(); it can be used to generate 2^128
 * non-overlapping subsequences for parallel computations.
 *
 * @param state State of the random number generator.
 */
void
rand_jump(struct rand_state* state);

#endif // GINC_ROGUELIKE_RAND_H
