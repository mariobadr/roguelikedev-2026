#include "world.h"

static void
carve_map(struct rl_tile_map* map, struct rl_layout const* layout)
{
  struct rl_tile tile = { 0 };
  tile.type = RL_TILE_FLOOR;

  for (int i = 0; i < layout->room_count; i++) {
    rl_fill_rect(map, &layout->rooms[i], tile);
  }

  for (int i = 0; i < layout->corridor_count; i++) {
    struct rl_corridor const* corridor = &layout->corridors[i];
    for (int j = 0; j < corridor->segment_count; j++) {
      rl_fill_rect(map, &corridor->segments[j], tile);
    }
  }
}

bool
rl_init_world(struct rl_world* world,
              int width,
              int height,
              struct rand_state* rng)

{
  if (!rl_init_map(&world->map, width, height)) {
    return false;
  }

  if (!rl_init_layout(&world->layout, width, height, rng)) {
    rl_free_world(world);
    return false;
  }

  carve_map(&world->map, &world->layout);

  return true;
}

void
rl_free_world(struct rl_world* world)
{
  if (world == NULL) {
    return;
  }

  rl_free_layout(&world->layout);
  rl_free_map(&world->map);
}
