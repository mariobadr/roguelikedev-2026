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
