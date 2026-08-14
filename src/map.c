#include "map.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_rect.h>

#include "bsp.h"
#include "rand.h"

static void
set_walls(struct rl_world_map* map, SDL_Rect rect)
{
  int left = rect.x;
  int right = rect.x + rect.w - 1;
  int top = rect.y;
  int bottom = rect.y + rect.h - 1;

  // top and bottom walls
  for (int x = left; x <= right; x++) {
    map->tiles[top * map->width + x].type = RL_TILE_WALL;
    map->tiles[bottom * map->width + x].type = RL_TILE_WALL;
  }

  // left and right walls (corners already handled above)
  for (int y = top + 1; y < bottom; y++) {
    map->tiles[y * map->width + left].type = RL_TILE_WALL;
    map->tiles[y * map->width + right].type = RL_TILE_WALL;
  }
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
      map->tiles[y * map->width + x].type = RL_TILE_FLOOR;
    }
  }

  struct rand_state rng;
  rand_seed(&rng, 1234);

  SDL_Rect rect = { 0 };
  rect.w = map->width;
  rect.h = map->height;

  struct rl_bsp_policy policy;
  policy.max_depth = 2;
  policy.min_size = 4;
  policy.stop_chance = 25;

  // generate the rooms
  struct rl_bsp_tree tree;
  rl_bsp_tree_init(&tree, rect);
  rl_bsp_split(&tree, 0, &rng, 0, &policy);

  // assign the rooms to the map
  for (int i = 0; i < RL_BSP_CAPACITY; i++) {
    struct rl_bsp_node const* node = &tree.nodes[i];
    if (node->axis == RL_BSP_SPLIT_NONE && node->rect.w > 0 &&
        node->rect.h > 0) {
      // this is a leaf node
      set_walls(map, node->rect);
    }
  }
}

struct rl_tile
rl_get_tile(struct rl_world_map const* map, int x, int y)
{
  return map->tiles[y * map->width + x];
}
