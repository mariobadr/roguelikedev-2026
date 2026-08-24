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
  // randomly generate the dungeon layout
  if (!rl_init_layout(&world->layout, width, height, rng)) {
    return false;
  }

  // allocate space for the tile map
  if (!rl_init_map(&world->map, width, height)) {
    rl_free_world(world);
    return false;
  }

  // update the tiles in the map based on the layout
  carve_map(&world->map, &world->layout);

  world->rogue = rl_create_entity(RL_ENTITY_ROGUE);

  // just put the rogue at the centre of the first room
  SDL_Rect const* room = &world->layout.rooms[0];
  world->rogue.position.x = room->x + room->w / 2;
  world->rogue.position.y = room->y + room->h / 2;

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

void
rl_update_world(struct rl_world* world, struct rl_command const* player_command)
{
  if (player_command->type == RL_COMMAND_NONE) {
    // if the player didn't take a turn, no other entity should either
    return;
  }

  if (player_command->type == RL_COMMAND_MOVE) {
    rl_move_entity(&world->rogue, &world->map, player_command->direction);
  }
}
