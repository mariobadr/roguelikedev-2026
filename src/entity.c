#include "entity.h"

#include "palette.h"
#include "tile_map.h"

struct rl_entity
rl_create_entity(enum rl_entity_type type)
{
  struct rl_entity entity = { 0 };

  switch (type) {
    case RL_ENTITY_RAT:
      entity.glyph = 'r';
      entity.colour = RL_COLOUR_YELLOW[5];
      entity.name = "Rat";
      break;
    case RL_ENTITY_ROGUE:
      entity.glyph = '@';
      entity.colour = RL_COLOUR_WHITE;
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
