/**
 * @file world.h
 */
#ifndef GINC_ROGUELIKE_SERIALIZE_WORLD_H
#define GINC_ROGUELIKE_SERIALIZE_WORLD_H

#include <SDL3/SDL_stdinc.h>

#include "game/serialize/result.h"

// external forward declarations
typedef struct SDL_IOStream SDL_IOStream;

// forward declarations
struct rl_world;

/**
 * Write world's state to dst.
 *
 * @return whether the write succeeded.
 */
bool
rl_write_world(SDL_IOStream* dst, struct rl_world const* world);

/**
 * Read a world's state from src into world, which must be allocated
 * beforehand (see rl_alloc_world).
 *
 * On failure, world may hold a partially-read state.
 */
enum rl_read_result
rl_read_world(SDL_IOStream* src, struct rl_world* world);

#endif // GINC_ROGUELIKE_SERIALIZE_WORLD_H
