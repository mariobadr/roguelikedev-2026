#include "generate.h"

#include "level.h"
#include "spawn.h"
#include "world.h"

static void
fill_map(grid(rl_tile) * map, SDL_Rect const* rect, enum rl_tile tile)
{
  for (int y = rect->y; y < rect->y + rect->h; ++y) {
    for (int x = rect->x; x < rect->x + rect->w; ++x) {
      *grid_at(map, x, y) = tile;
    }
  }
}

static void
carve_map(grid(rl_tile) * map, struct rl_layout const* layout)
{
  enum rl_tile tile = RL_TILE_FLOOR;

  for (int i = 0; i < array_len(&layout->rooms); i++) {
    fill_map(map, array_at(&layout->rooms, i), tile);
  }

  for (int i = 0; i < array_len(&layout->corridors); i++) {
    struct rl_corridor const* corridor = array_at(&layout->corridors, i);
    for (int j = 0; j < corridor->segment_count; j++) {
      fill_map(map, &corridor->segments[j], tile);
    }
  }
}

static bool
generate_level(struct rl_level* level, struct rand_state* rng)
{
  int const width = grid_width(&level->map);
  int const height = grid_height(&level->map);

  // randomly generate the dungeon layout
  if (!rl_init_layout(&level->layout, width, height, rng)) {
    return false;
  }

  // update the tiles in the map based on the layout
  carve_map(&level->map, &level->layout);

  return true;
}

static bool
populate_level(struct rl_world* world,
               struct rl_level* level,
               struct rand_state* rng)
{
  // put the rogue at the centre of the first room
  int const rogue_room = 0;
  SDL_Rect const* room = array_at(&level->layout.rooms, rogue_room);
  world->rogue.pos.x = room->x + room->w / 2;
  world->rogue.pos.y = room->y + room->h / 2;

  // spawn the other actors
  if (!rl_spawn_actors(level, &world->actors, rogue_room, rng)) {
    return false;
  }

  // spawn items
  if (!rl_spawn_items(level, &world->items, rng)) {
    return false;
  }

  return true;
}

bool
rl_gen_level(struct rl_world* world,
             struct rl_level* level,
             struct rand_state* rng)
{
  if (!generate_level(level, rng)) {
    return false;
  }

  if (!populate_level(world, level, rng)) {
    return false;
  }

  // TODO: push level into world

  return true;
}
