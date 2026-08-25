#include "entity.h"

#include "palette.h"
#include "tile_map.h"

struct rl_entity
rl_create_entity(enum rl_entity_type type)
{
  struct rl_entity entity = { 0 };

  switch (type) {
    case RL_ENTITY_RAT:
      entity.type = RL_ENTITY_RAT;
      entity.name = "Rat";
      break;
    case RL_ENTITY_ROGUE:
      entity.type = RL_ENTITY_ROGUE;
      entity.name = "Rogue";
      break;
    default:
      break;
  }

  return entity;
}

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
