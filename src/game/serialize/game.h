/**
 * @file game.h
 */
#ifndef GINC_ROGUELIKE_SERIALIZE_GAME_H
#define GINC_ROGUELIKE_SERIALIZE_GAME_H

#include <SDL3/SDL_stdinc.h>

#include "game/serialize/result.h"

// external forward declarations
typedef struct SDL_IOStream SDL_IOStream;

// forward declarations
struct rl_game;

/**
 * Write game's state to dst.
 *
 * @return whether the write succeeded.
 */
bool
rl_write_game(SDL_IOStream* dst, struct rl_game const* game);

/**
 * Read a game's state from src into game, which must be allocated
 * beforehand (see rl_alloc_game).
 *
 * On success, src is left positioned right after the game record; it is
 * the caller's responsibility to check for trailing data if the format
 * composing this record requires it to be the last thing in the stream.
 *
 * On failure, game may hold a partially-read state.
 */
enum rl_read_result
rl_read_game(SDL_IOStream* src, struct rl_game* game);

#endif // GINC_ROGUELIKE_SERIALIZE_GAME_H
