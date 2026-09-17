#include "level.h"

#include <SDL3/SDL_iostream.h>

#include "result.h"
#include "context.h"

#include "game/level.h"

static bool
write_map_grid(SDL_IOStream* dst, struct rl_level const* level)
{
  int const width = grid_width(&level->map);
  int const height = grid_height(&level->map);

  bool ok = true;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      ok &= SDL_WriteU8(dst, (Uint8)(*grid_at(&level->map, x, y)));
    }
  }

  return ok;
}

static bool
write_explored_grid(SDL_IOStream* dst, struct rl_level const* level)
{
  int const width = grid_width(&level->map);
  int const height = grid_height(&level->map);

  bool ok = true;

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      ok &= SDL_WriteU8(dst, *grid_at(&level->explored, x, y) ? 1 : 0);
    }
  }

  return ok;
}

bool
rl_write_level(SDL_IOStream* dst,
               struct rl_writer const* w,
               struct rl_level const* level)
{
  int const width = grid_width(&level->map);
  int const height = grid_height(&level->map);

  bool ok = true;
  ok &= SDL_WriteS32LE(dst, (Sint32)level->depth);
  ok &= SDL_WriteS32LE(dst, (Sint32)width);
  ok &= SDL_WriteS32LE(dst, (Sint32)height);

  ok &= write_map_grid(dst, level);
  ok &= write_explored_grid(dst, level);

  Uint32 const n = (Uint32)alist_len(&level->actors);
  ok &= SDL_WriteU32LE(dst, n);
  for (Uint32 i = 0; i < n; i++) {
    Uint32 const id = rl_to_actor_id(w, *alist_at(&level->actors, i));
    ok &= id != 0;
    ok &= SDL_WriteU32LE(dst, id);
  }

  Uint32 const m = (Uint32)alist_len(&level->items);
  ok &= SDL_WriteU32LE(dst, m);
  for (Uint32 i = 0; i < m; i++) {
    Uint32 const id = rl_to_item_id(w, *alist_at(&level->items, i));
    ok &= id != 0;
    ok &= SDL_WriteU32LE(dst, id);
  }

  return ok;
}

static bool
is_valid_tile(Uint8 value)
{
  switch ((enum rl_tile)value) {
    case RL_TILE_WALL:
    case RL_TILE_FLOOR:
      return true;
  }

  return false;
}

static enum rl_read_result
read_map_grid(SDL_IOStream* src, struct rl_level* level)
{
  int const width = grid_width(&level->map);
  int const height = grid_height(&level->map);

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      Uint8 value = 0;
      RL_READ_OR_FAIL(src, SDL_ReadU8(src, &value));

      if (!is_valid_tile(value)) {
        return RL_READ_CORRUPT;
      }
      *grid_at(&level->map, x, y) = (enum rl_tile)value;
    }
  }

  return RL_READ_OK;
}

static enum rl_read_result
read_explored_grid(SDL_IOStream* src, struct rl_level* level)
{
  int const width = grid_width(&level->map);
  int const height = grid_height(&level->map);

  for (int y = 0; y < height; y++) {
    for (int x = 0; x < width; x++) {
      Uint8 value = 0;
      RL_READ_OR_FAIL(src, SDL_ReadU8(src, &value));

      if (value > 1) {
        return RL_READ_CORRUPT;
      }
      *grid_at(&level->explored, x, y) = value != 0;
    }
  }

  return RL_READ_OK;
}

static enum rl_read_result
read_level_actors(SDL_IOStream* src,
                  struct rl_reader const* r,
                  struct rl_level* level)
{
  Uint32 count = 0;
  RL_READ_OR_FAIL(src, SDL_ReadU32LE(src, &count));

  for (Uint32 i = 0; i < count; i++) {
    Uint32 actor_id = 0;
    RL_READ_OR_FAIL(src, SDL_ReadU32LE(src, &actor_id));

    if (actor_id < 1 || actor_id > r->actor_count) {
      return RL_READ_CORRUPT;
    }

    handle(rl_actor) const h = *array_at(&r->actor_handles, actor_id);

    if (!rl_add_actor(level, h)) {
      return RL_READ_ERROR;
    }
  }

  return RL_READ_OK;
}

static enum rl_read_result
read_level_items(SDL_IOStream* src,
                 struct rl_reader const* r,
                 struct rl_level* level)
{
  Uint32 count = 0;
  RL_READ_OR_FAIL(src, SDL_ReadU32LE(src, &count));

  for (Uint32 i = 0; i < count; i++) {
    Uint32 item_id = 0;
    RL_READ_OR_FAIL(src, SDL_ReadU32LE(src, &item_id));

    if (item_id < 1 || item_id > r->item_count) {
      return RL_READ_CORRUPT;
    }

    handle(rl_item) const h = *array_at(&r->item_handles, item_id);

    for (size_t j = 0; j < alist_len(&level->items); j++) {
      if (handle_equal(*alist_at(&level->items, j), h)) {
        return RL_READ_CORRUPT;
      }
    }

    if (!rl_add_item(level, h)) {
      return RL_READ_ERROR;
    }
  }

  return RL_READ_OK;
}

enum rl_read_result
rl_read_level(SDL_IOStream* src,
              struct rl_reader const* r,
              struct rl_level* out)
{
  Sint32 depth = 0;
  RL_READ_OR_FAIL(src, SDL_ReadS32LE(src, &depth));

  Sint32 width = 0;
  RL_READ_OR_FAIL(src, SDL_ReadS32LE(src, &width));

  Sint32 height = 0;
  RL_READ_OR_FAIL(src, SDL_ReadS32LE(src, &height));

  if (width < 1 || width > RL_SNAPSHOT_MAX_DIM || height < 1 ||
      height > RL_SNAPSHOT_MAX_DIM) {
    return RL_READ_CORRUPT;
  }

  if (!rl_alloc_level(out, (int)depth, (int)width, (int)height)) {
    return RL_READ_ERROR;
  }

  enum rl_read_result result = read_map_grid(src, out);
  if (result == RL_READ_OK) {
    result = read_explored_grid(src, out);
  }

  if (result != RL_READ_OK) {
    return result;
  }

  result = read_level_actors(src, r, out);
  if (result != RL_READ_OK) {
    return result;
  }

  return read_level_items(src, r, out);
}
