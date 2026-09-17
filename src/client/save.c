#include "save.h"

#include <SDL3/SDL_filesystem.h>
#include <SDL3/SDL_iostream.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>

#include "save_format.h"

#define RL_SAVE_EXTENSION ".sav"

static char*
get_save_dir(void)
{
  char* dir = SDL_GetPrefPath("Professor Mario", "A Roguelike");
  if (dir == NULL) {
    SDL_Log("SDL_GetPrefPath failed: %s", SDL_GetError());
  }

  return dir;
}

static char*
get_save_path(struct rl_save_id id)
{
  char* dir = get_save_dir();
  if (dir == NULL) {
    return NULL;
  }

  char* path = NULL;
  int const result = SDL_asprintf(
    &path, "%s%016" SDL_PRIx64 "%s", dir, id.value, RL_SAVE_EXTENSION);
  if (result < 0) {
    SDL_Log("SDL_asprintf failed: %s", SDL_GetError());
    path = NULL;
  }

  SDL_free(dir);

  return path;
}

static bool
write_temp_save(char const* tmp_path,
                 struct rl_save_info const* info,
                 struct rl_game const* game)
{
  SDL_IOStream* io = SDL_IOFromFile(tmp_path, "wb");
  if (io == NULL) {
    SDL_Log("SDL_IOFromFile failed: %s", SDL_GetError());
    return false;
  }

  bool ok = rl_write_save(io, info, game) == RL_SAVE_OK;

  bool const closed = SDL_CloseIO(io);
  ok = ok && closed;

  if (!ok) {
    SDL_Log("failed to write save: %s", SDL_GetError());
    SDL_RemovePath(tmp_path);
  }

  return ok;
}

static enum rl_save_result
write_save(struct rl_game const* game, struct rl_save_info const* info)
{
  char* path = get_save_path(info->id);
  if (path == NULL) {
    return RL_SAVE_ERROR;
  }

  char* tmp_path = NULL;
  if (SDL_asprintf(&tmp_path, "%s.tmp", path) < 0) {
    SDL_Log("SDL_asprintf failed: %s", SDL_GetError());
    SDL_free(path);
    return RL_SAVE_ERROR;
  }

  enum rl_save_result result = RL_SAVE_ERROR;

  if (write_temp_save(tmp_path, info, game)) {
    if (SDL_RenamePath(tmp_path, path)) {
      result = RL_SAVE_OK;
    } else {
      SDL_Log("SDL_RenamePath failed: %s", SDL_GetError());
      SDL_RemovePath(tmp_path);
    }
  }

  SDL_free(tmp_path);
  SDL_free(path);

  return result;
}

static bool
has_save_extension(char const* fname)
{
  size_t const len = SDL_strlen(fname);
  size_t const ext_len = SDL_strlen(RL_SAVE_EXTENSION);

  if (len <= ext_len) {
    return false;
  }

  return SDL_strcasecmp(fname + (len - ext_len), RL_SAVE_EXTENSION) == 0;
}

static SDL_EnumerationResult
list_saves_callback(void* userdata, char const* dirname, char const* fname)
{
  alist(rl_save_info)* entries = userdata;

  if (!has_save_extension(fname)) {
    return SDL_ENUM_CONTINUE;
  }

  struct rl_save_id id = { 0 };
  id.value = (Uint64)SDL_strtoull(fname, NULL, 16);

  char* path = NULL;
  if (SDL_asprintf(&path, "%s%s", dirname, fname) < 0) {
    SDL_Log("SDL_asprintf failed: %s", SDL_GetError());
    return SDL_ENUM_FAILURE;
  }

  struct rl_save_info* entry = alist_push(entries);
  if (entry == NULL) {
    SDL_free(path);
    return SDL_ENUM_FAILURE;
  }

  SDL_zerop(entry);
  entry->id = id;
  entry->condition = RL_SAVE_CONDITION_CORRUPT;

  SDL_IOStream* io = SDL_IOFromFile(path, "rb");
  SDL_free(path);

  if (io != NULL) {
    rl_read_save_summary(io, entry);
    SDL_CloseIO(io);
  }

  return SDL_ENUM_CONTINUE;
}

static int
compare_save_info_by_recency(void const* a, void const* b)
{
  struct rl_save_info const* lhs = a;
  struct rl_save_info const* rhs = b;

  if (lhs->saved_at > rhs->saved_at) {
    return -1;
  }

  if (lhs->saved_at < rhs->saved_at) {
    return 1;
  }

  return 0;
}

bool
rl_list_saves(alist(rl_save_info) * out)
{
  alist_clear(out);

  char* dir = get_save_dir();
  if (dir == NULL) {
    return false;
  }

  bool const enumerated =
    SDL_EnumerateDirectory(dir, list_saves_callback, out);
  SDL_free(dir);

  if (!enumerated) {
    SDL_Log("SDL_EnumerateDirectory failed: %s", SDL_GetError());
    alist_clear(out);
    return false;
  }

  SDL_qsort(
    out->data, out->len, sizeof(*out->data), compare_save_info_by_recency);

  return true;
}

enum rl_save_result
rl_create_save(struct rl_game const* game, struct rl_save_id* out_id)
{
  SDL_Time now = 0;
  if (!SDL_GetCurrentTime(&now)) {
    SDL_Log("SDL_GetCurrentTime failed: %s", SDL_GetError());
    return RL_SAVE_ERROR;
  }

  struct rl_save_id id = { 0 };
  id.value = (Uint64)now;

  struct rl_save_info const info =
    rl_summarise_save(game, id, RL_RUN_ACTIVE, now);

  enum rl_save_result const result = write_save(game, &info);
  if (result != RL_SAVE_OK) {
    return result;
  }

  *out_id = id;
  return RL_SAVE_OK;
}

static enum rl_save_result
read_existing_outcome(struct rl_save_id id, enum rl_run_outcome* out_outcome)
{
  char* path = get_save_path(id);
  if (path == NULL) {
    return RL_SAVE_ERROR;
  }

  SDL_IOStream* io = SDL_IOFromFile(path, "rb");
  SDL_free(path);

  if (io == NULL) {
    return RL_SAVE_MISSING;
  }

  struct rl_save_info existing = { 0 };
  existing.id = id;
  enum rl_save_result const lookup = rl_read_save_summary(io, &existing);
  SDL_CloseIO(io);

  *out_outcome = (lookup == RL_SAVE_OK) ? existing.outcome : RL_RUN_ACTIVE;

  return RL_SAVE_OK;
}

enum rl_save_result
rl_save_game(struct rl_save_id id, struct rl_game const* game)
{
  if (!rl_save_id_is_valid(id)) {
    return RL_SAVE_MISSING;
  }

  enum rl_run_outcome outcome = RL_RUN_ACTIVE;
  enum rl_save_result const lookup = read_existing_outcome(id, &outcome);
  if (lookup != RL_SAVE_OK) {
    return lookup;
  }

  SDL_Time now = 0;
  if (!SDL_GetCurrentTime(&now)) {
    SDL_Log("SDL_GetCurrentTime failed: %s", SDL_GetError());
    return RL_SAVE_ERROR;
  }

  struct rl_save_info const info = rl_summarise_save(game, id, outcome, now);

  return write_save(game, &info);
}

enum rl_save_result
rl_load_game(struct rl_save_id id, struct rl_game* game)
{
  if (!rl_save_id_is_valid(id)) {
    return RL_SAVE_MISSING;
  }

  char* path = get_save_path(id);
  if (path == NULL) {
    return RL_SAVE_ERROR;
  }

  SDL_IOStream* io = SDL_IOFromFile(path, "rb");
  SDL_free(path);

  if (io == NULL) {
    return RL_SAVE_MISSING;
  }

  struct rl_save_info info = { 0 };
  info.id = id;
  enum rl_save_result const result = rl_read_save(io, &info, game);

  SDL_CloseIO(io);

  return result;
}

enum rl_save_result
rl_finish_save(struct rl_save_id id, enum rl_run_outcome outcome)
{
  if (!rl_save_id_is_valid(id)) {
    return RL_SAVE_MISSING;
  }

  char* path = get_save_path(id);
  if (path == NULL) {
    return RL_SAVE_ERROR;
  }

  SDL_IOStream* io = SDL_IOFromFile(path, "r+b");
  SDL_free(path);

  if (io == NULL) {
    return RL_SAVE_MISSING;
  }

  enum rl_save_result result = rl_patch_save_outcome(io, outcome);

  bool const closed = SDL_CloseIO(io);
  if (result == RL_SAVE_OK && !closed) {
    result = RL_SAVE_ERROR;
  }

  return result;
}

enum rl_save_result
rl_delete_save(struct rl_save_id id)
{
  if (!rl_save_id_is_valid(id)) {
    return RL_SAVE_OK;
  }

  char* path = get_save_path(id);
  if (path == NULL) {
    return RL_SAVE_ERROR;
  }

  bool const exists = SDL_GetPathInfo(path, NULL);
  if (!exists) {
    SDL_free(path);
    return RL_SAVE_OK;
  }

  bool const ok = SDL_RemovePath(path);
  if (!ok) {
    SDL_Log("SDL_RemovePath failed: %s", SDL_GetError());
  }

  SDL_free(path);

  return ok ? RL_SAVE_OK : RL_SAVE_ERROR;
}
