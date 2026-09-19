/**
 * @file level.h
 */
#ifndef GINC_ROGUELIKE_SAVE_LEVEL_H
#define GINC_ROGUELIKE_SAVE_LEVEL_H

#include <SDL3/SDL_stdinc.h>

#include "save/result.h"

// external forward declarations
typedef struct SDL_IOStream SDL_IOStream;

// forward declarations
struct rl_level;
struct rl_reader;
struct rl_writer;

/**
 * Write level's state to dst.
 *
 * @param w resolves the level's actor and item handles to stable ids.
 *
 * @return whether the write succeeded.
 */
bool
rl_write_level(SDL_IOStream* dst,
               struct rl_writer const* w,
               struct rl_level const* level);

/**
 * Read a level's state from src into out, which must be zeroed.
 *
 * On failure, out may hold a partially-read level.
 *
 * @param r resolves stable actor and item ids back to handles.
 */
enum rl_read_result
rl_read_level(SDL_IOStream* src,
              struct rl_reader const* r,
              struct rl_level* out);

#endif // GINC_ROGUELIKE_SAVE_LEVEL_H
