#include "entity.h"

#include "tile_map.h"

bool
rl_move_entity(struct rl_entity* entity,
               struct rl_tile_map const* map,
               SDL_Point move_vector)
{
  SDL_Point dst = { 0 };
  dst.x = entity->position.x + move_vector.x;
  dst.y = entity->position.y + move_vector.y;

  if (rl_is_walkable(rl_get_tile(map, dst.x, dst.y))) {
    entity->position = dst;
    return true;
  }

  return false;
}
