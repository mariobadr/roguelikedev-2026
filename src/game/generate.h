/**
 * @file generate.h
 */
#ifndef GINC_ROGUELIKE_GENERATE_H
#define GINC_ROGUELIKE_GENERATE_H

#include <SDL3/SDL_stdinc.h>

// forward declarations
struct rand_state;
struct rl_level;
struct rl_world;

bool
rl_gen_level(struct rl_world* world,
             struct rl_level* level,
             struct rand_state* rng);

#endif // GINC_ROGUELIKE_GENERATE_H
