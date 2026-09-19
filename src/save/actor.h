/**
 * @file actor.h
 */
#ifndef GINC_ROGUELIKE_SAVE_ACTOR_H
#define GINC_ROGUELIKE_SAVE_ACTOR_H

#include <SDL3/SDL_stdinc.h>

#include "save/result.h"

// external forward declarations
typedef struct SDL_IOStream SDL_IOStream;

// forward declarations
struct rl_actor;

/**
 * Write actor's state to dst.
 *
 * @return whether the write succeeded.
 */
bool
rl_write_actor(SDL_IOStream* dst, struct rl_actor const* actor);

/**
 * Read an actor's state from src into out.
 *
 * The out parameter is left unmodified on failure.
 */
enum rl_read_result
rl_read_actor(SDL_IOStream* src, struct rl_actor* out);

#endif // GINC_ROGUELIKE_SAVE_ACTOR_H
