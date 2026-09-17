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
 * Read a complete, playable game from src into out.
 *
 * @param out must be zero-initialized.
 */
enum rl_read_result
rl_read_game(SDL_IOStream* src, struct rl_game* out);

#endif // GINC_ROGUELIKE_SERIALIZE_GAME_H
