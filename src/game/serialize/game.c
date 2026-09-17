#include "game.h"

#include <SDL3/SDL_iostream.h>

#include "result.h"
#include "world.h"

#include "game/game.h"

bool
rl_write_game(SDL_IOStream* dst, struct rl_game const* game)
{
  bool ok = SDL_WriteU64LE(dst, game->turns);
  for (int i = 0; i < 4; i++) {
    ok &= SDL_WriteU64LE(dst, game->rng.s[i]);
  }

  ok &= rl_write_world(dst, &game->world);

  return ok;
}

static enum rl_read_result
read_snapshot_header(SDL_IOStream* src, struct rl_game* game)
{
  RL_READ_OR_FAIL(src, SDL_ReadU64LE(src, &game->turns));

  for (int i = 0; i < 4; i++) {
    RL_READ_OR_FAIL(src, SDL_ReadU64LE(src, &game->rng.s[i]));
  }

  if (game->rng.s[0] == 0 && game->rng.s[1] == 0 && game->rng.s[2] == 0 &&
      game->rng.s[3] == 0) {
    return RL_READ_CORRUPT;
  }

  return RL_READ_OK;
}

enum rl_read_result
rl_read_game(SDL_IOStream* src, struct rl_game* out)
{
  struct rl_game tmp = { 0 };

  if (!rl_alloc_world(&tmp.world)) {
    return RL_READ_ERROR;
  }

  enum rl_read_result result = read_snapshot_header(src, &tmp);
  if (result == RL_READ_OK) {
    result = rl_read_world(src, &tmp.world);
  }

  if (result == RL_READ_OK && !rl_prepare_game(&tmp)) {
    result = RL_READ_ERROR;
  }

  if (result != RL_READ_OK) {
    rl_free_game(&tmp);
    return result;
  }

  *out = tmp;
  return RL_READ_OK;
}
