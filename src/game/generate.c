#include "generate.h"

#include "procgen/layout.h"

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
generate_level(struct rl_level* level,
               struct rl_layout* layout,
               struct rand_state* rng)
{
  int const width = grid_width(&level->map);
  int const height = grid_height(&level->map);

  // randomly generate the dungeon layout
  if (!rl_init_layout(layout, width, height, rng)) {
    return false;
  }

  // update the tiles in the map based on the layout
  carve_map(&level->map, layout);

  return true;
}

static bool
populate_level(struct rl_world* world,
               struct rl_level* level,
               struct rl_layout const* layout,
               struct rand_state* rng)
{
  // put the rogue at the centre of the first room
  int const rogue_room = 0;
  SDL_Rect const* room = array_at(&layout->rooms, rogue_room);
  struct rl_actor* rogue = rl_borrow_mut_actor(world, rl_get_rogue(world));
  rogue->pos.x = room->x + room->w / 2;
  rogue->pos.y = room->y + room->h / 2;

  // spawn the other actors (invalidates rogue if world->actors grows)
  if (!rl_spawn_actors(level, layout, world, rogue_room, rng)) {
    return false;
  }

  // spawn items
  if (!rl_spawn_items(level, layout, &world->items, rng)) {
    return false;
  }

  return true;
}

bool
rl_gen_level(struct rl_world* world,
             struct rl_level* level,
             struct rand_state* rng)
{
  // the layout only matters while generating -- it's not kept afterward
  struct rl_layout layout = { 0 };

  bool ok = generate_level(level, &layout, rng);
  if (ok) {
    ok = populate_level(world, level, &layout, rng);
  }

  rl_free_layout(&layout);

  return ok;
}
