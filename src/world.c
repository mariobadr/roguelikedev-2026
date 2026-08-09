#include "world.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_rect.h>

static void
generate_room(struct rl_world_map* map)
{
  SDL_Rect room = { 0 };

  // randomly decide a room size
  room.w = SDL_rand(map->width - 3) + 3;
  room.h = SDL_rand(map->height - 3) + 3;

  // randomly decide where the room goes
  room.x = SDL_rand(map->width - room.w);
  room.y = SDL_rand(map->height - room.h);

  int left = room.x;
  int right = room.x + room.w - 1;
  int top = room.y;
  int bottom = room.y + room.h - 1;

  // top and bottom walls
  for (int x = left; x <= right; x++) {
    map->tiles[top * map->width + x].walkable = false;
    map->tiles[bottom * map->width + x].walkable = false;
  }

  // left and right walls (corners already handled above)
  for (int y = top + 1; y < bottom; y++) {
    map->tiles[y * map->width + left].walkable = false;
    map->tiles[y * map->width + right].walkable = false;
  }

  // add a "door" at some random spot on the left-hand side
  int door_y = top + 1 + SDL_rand(room.h - 2);
  map->tiles[door_y * map->width + left].walkable = true;
}

bool
rl_init_map(struct rl_world_map* map, int width, int height)
{
  // check whether map has already been initialized
  SDL_assert(map->tiles == NULL);

  map->tiles = SDL_calloc(width * height, sizeof(*map->tiles));
  if (map->tiles == NULL) {
    SDL_Log("SDL_calloc failed: %s", SDL_GetError());
    return false;
  }

  map->width = width;
  map->height = height;

  return true;
}

void
rl_generate_map(struct rl_world_map* map)
{
  // note: this is all temporary

  // start everything off as walkable for now
  for (int y = 0; y < map->height; y++) {
    for (int x = 0; x < map->width; x++) {
      map->tiles[y * map->width + x].walkable = true;
    }
  }

  generate_room(map);
}

struct rl_world_tile
rl_get_tile(struct rl_world_map const* map, int x, int y)
{
  return map->tiles[y * map->width + x];
}
