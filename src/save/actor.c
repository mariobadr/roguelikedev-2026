#include "actor.h"

#include <SDL3/SDL_iostream.h>

#include "result.h"

#include "game/actor.h"

static bool
is_valid_actor_type(enum rl_actor_type type)
{
  switch (type) {
    case RL_ACTOR_ROGUE:
    case RL_ACTOR_RAT:
      return true;
  }

  return false;
}

bool
rl_write_actor(SDL_IOStream* dst, struct rl_actor const* actor)
{
  bool ok = true;
  ok &= SDL_WriteU32LE(dst, (Uint32)actor->type);
  ok &= SDL_WriteS32LE(dst, (Sint32)actor->level);
  ok &= SDL_WriteS32LE(dst, (Sint32)actor->pos.x);
  ok &= SDL_WriteS32LE(dst, (Sint32)actor->pos.y);
  ok &= SDL_WriteU8(dst, actor->awake ? 1 : 0);
  ok &= SDL_WriteS32LE(dst, (Sint32)actor->hp);
  ok &= SDL_WriteS32LE(dst, (Sint32)actor->stats.max_hp);
  ok &= SDL_WriteS32LE(dst, (Sint32)actor->stats.strength);
  ok &= SDL_WriteS32LE(dst, (Sint32)actor->stats.armor);

  return ok;
}

enum rl_read_result
rl_read_actor(SDL_IOStream* src, struct rl_actor* out)
{
  Uint32 type_value = 0;
  RL_READ_OR_FAIL(src, SDL_ReadU32LE(src, &type_value));
  if (!is_valid_actor_type((enum rl_actor_type)type_value)) {
    return RL_READ_CORRUPT;
  }

  Sint32 level = 0;
  RL_READ_OR_FAIL(src, SDL_ReadS32LE(src, &level));
  if (level < 1) {
    return RL_READ_CORRUPT;
  }

  Sint32 x = 0;
  Sint32 y = 0;
  RL_READ_OR_FAIL(src, SDL_ReadS32LE(src, &x));
  RL_READ_OR_FAIL(src, SDL_ReadS32LE(src, &y));

  Uint8 awake = 0;
  RL_READ_OR_FAIL(src, SDL_ReadU8(src, &awake));

  Sint32 hp = 0;
  Sint32 max_hp = 0;
  Sint32 strength = 0;
  Sint32 armor = 0;
  RL_READ_OR_FAIL(src, SDL_ReadS32LE(src, &hp));
  RL_READ_OR_FAIL(src, SDL_ReadS32LE(src, &max_hp));
  RL_READ_OR_FAIL(src, SDL_ReadS32LE(src, &strength));
  RL_READ_OR_FAIL(src, SDL_ReadS32LE(src, &armor));

  if (max_hp <= 0 || hp > max_hp) {
    return RL_READ_CORRUPT;
  }

  out->type = (enum rl_actor_type)type_value;
  out->level = (int)level;
  out->pos.x = (int)x;
  out->pos.y = (int)y;
  out->awake = awake != 0;
  out->hp = (int)hp;
  out->stats.max_hp = (int)max_hp;
  out->stats.strength = (int)strength;
  out->stats.armor = (int)armor;

  return RL_READ_OK;
}
