#include "save_format.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>

#include "game/game.h"
#include "game/serialize/game.h"

#define RL_SAVE_MAGIC 0x564C5352u // "RLSV"
#define RL_SAVE_VERSION 3u

// offset of the outcome field within the header written by write_header;
// used by rl_patch_save_outcome to patch it in place without disturbing the
// rest of the file. write_header asserts this offset via SDL_TellIO.
#define RL_SAVE_OUTCOME_OFFSET 16

static bool
write_header(SDL_IOStream* dst, struct rl_save_info const* info)
{
  bool ok = true;

  ok &= SDL_WriteU32LE(dst, RL_SAVE_MAGIC);
  ok &= SDL_WriteU32LE(dst, RL_SAVE_VERSION);
  ok &= SDL_WriteU64LE(dst, info->id.value);

  Sint64 const outcome_offset = SDL_TellIO(dst);
  SDL_assert(outcome_offset < 0 || outcome_offset == RL_SAVE_OUTCOME_OFFSET);
  ok &= SDL_WriteU32LE(dst, (Uint32)info->outcome);
  ok &= SDL_WriteS64LE(dst, info->saved_at);
  ok &= SDL_WriteU64LE(dst, info->turns);
  ok &= SDL_WriteS32LE(dst, (Sint32)info->depth);
  ok &= SDL_WriteS32LE(dst, (Sint32)info->hp);
  ok &= SDL_WriteS32LE(dst, (Sint32)info->max_hp);

  return ok;
}

static enum rl_save_result
read_header(SDL_IOStream* src, struct rl_save_info* out)
{
  out->condition = RL_SAVE_CONDITION_CORRUPT;
  out->outcome = RL_RUN_ACTIVE;
  out->version = 0;
  out->saved_at = 0;
  out->turns = 0;
  out->depth = 0;
  out->hp = 0;
  out->max_hp = 0;

  bool ok = true;

  Uint32 magic = 0;
  ok &= SDL_ReadU32LE(src, &magic);

  Uint32 version = 0;
  ok &= SDL_ReadU32LE(src, &version);

  Uint64 id_value = 0;
  ok &= SDL_ReadU64LE(src, &id_value);

  Uint32 outcome = 0;
  ok &= SDL_ReadU32LE(src, &outcome);

  Sint64 saved_at = 0;
  ok &= SDL_ReadS64LE(src, &saved_at);

  Uint64 turns = 0;
  ok &= SDL_ReadU64LE(src, &turns);

  Sint32 depth = 0;
  ok &= SDL_ReadS32LE(src, &depth);

  Sint32 hp = 0;
  ok &= SDL_ReadS32LE(src, &hp);

  Sint32 max_hp = 0;
  ok &= SDL_ReadS32LE(src, &max_hp);

  // the save's identity comes from its filename, not its contents
  (void)id_value;

  if (!ok || magic != RL_SAVE_MAGIC) {
    return RL_SAVE_CORRUPT;
  }

  out->version = version;

  if (version != RL_SAVE_VERSION) {
    out->condition = RL_SAVE_CONDITION_INCOMPATIBLE;
    return RL_SAVE_INCOMPATIBLE;
  }

  if (outcome > RL_RUN_VICTORY) {
    return RL_SAVE_CORRUPT;
  }

  out->condition = RL_SAVE_CONDITION_OK;
  out->outcome = (enum rl_run_outcome)outcome;
  out->saved_at = (SDL_Time)saved_at;
  out->turns = turns;
  out->depth = (int)depth;
  out->hp = (int)hp;
  out->max_hp = (int)max_hp;

  return RL_SAVE_OK;
}

struct rl_save_info
rl_summarise_save(struct rl_game const* game,
                  struct rl_save_id id,
                  enum rl_run_outcome outcome,
                  SDL_Time saved_at)
{
  struct rl_actor const* rogue =
    rl_borrow_actor(&game->world, rl_get_rogue(&game->world));
  struct rl_level const* level = rl_get_current_level(&game->world);

  struct rl_save_info info = { 0 };
  info.id = id;
  info.condition = RL_SAVE_CONDITION_OK;
  info.outcome = outcome;
  info.version = RL_SAVE_VERSION;
  info.saved_at = saved_at;
  info.turns = game->turns;
  info.depth = level->depth;
  info.hp = rogue->hp;
  info.max_hp = rogue->max_hp;

  return info;
}

enum rl_save_result
rl_write_save(SDL_IOStream* dst,
              struct rl_save_info const* info,
              struct rl_game const* game)
{
  bool ok = write_header(dst, info);
  ok = ok && rl_write_game(dst, game);

  return ok ? RL_SAVE_OK : RL_SAVE_ERROR;
}

enum rl_save_result
rl_read_save_summary(SDL_IOStream* src, struct rl_save_info* out)
{
  return read_header(src, out);
}

enum rl_save_result
rl_read_save(SDL_IOStream* src,
             struct rl_save_info* out_info,
             struct rl_game* game)
{
  enum rl_save_result const header_result = read_header(src, out_info);
  if (header_result != RL_SAVE_OK) {
    return header_result;
  }

  if (out_info->outcome != RL_RUN_ACTIVE) {
    return RL_SAVE_UNAVAILABLE;
  }

  struct rl_game loaded = { 0 };
  enum rl_save_result result = RL_SAVE_ERROR;

  switch (rl_read_game(src, &loaded)) {
    case RL_READ_OK: {
      // the game record must be the last thing in the file
      Uint8 trailing = 0;
      if (SDL_ReadU8(src, &trailing)) {
        result = RL_SAVE_CORRUPT;
      } else if (SDL_GetIOStatus(src) != SDL_IO_STATUS_EOF) {
        result = RL_SAVE_ERROR;
      } else {
        result = RL_SAVE_OK;
      }
      break;
    }
    case RL_READ_CORRUPT:
      result = RL_SAVE_CORRUPT;
      break;
    case RL_READ_ERROR:
    default:
      result = RL_SAVE_ERROR;
      break;
  }

  if (result == RL_SAVE_OK) {
    *game = loaded;
  } else {
    rl_free_game(&loaded);
  }

  return result;
}

enum rl_save_result
rl_patch_save_outcome(SDL_IOStream* io, enum rl_run_outcome outcome)
{
  if (outcome != RL_RUN_DEAD && outcome != RL_RUN_VICTORY) {
    SDL_InvalidParamError("outcome");
    return RL_SAVE_ERROR;
  }

  struct rl_save_info info = { 0 };
  enum rl_save_result result = read_header(io, &info);

  if (result != RL_SAVE_OK) {
    // nothing to patch
    return result;
  }

  if (info.outcome != RL_RUN_ACTIVE) {
    // already ended; patching an ended run succeeds without changing it
    return RL_SAVE_OK;
  }

  bool ok = SDL_SeekIO(io, RL_SAVE_OUTCOME_OFFSET, SDL_IO_SEEK_SET) >= 0;
  if (!ok) {
    SDL_Log("SDL_SeekIO failed: %s", SDL_GetError());
  }

  ok = ok && SDL_WriteU32LE(io, (Uint32)outcome);

  return ok ? RL_SAVE_OK : RL_SAVE_ERROR;
}
