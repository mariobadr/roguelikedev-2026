#include "world.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "rand.h"

#define MAX_ENTITIES 32

static void
carve_map(struct rl_tile_map* map, struct rl_layout const* layout)
{
  enum rl_tile tile = RL_TILE_FLOOR;

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

static struct rl_entity*
find_entity(struct rl_world const *world, SDL_Point position)
{
  if (world->rogue.position.x == position.x &&
      world->rogue.position.y == position.y) {
    // casting away const - look away!
    return (struct rl_entity*)&world->rogue;
  }

  for (int i = 0; i < world->entity_count; i++) {
    struct rl_entity * entity = &world->entities[i];
    if (entity->position.x == position.x && entity->position.y == position.y) {
      return entity;
    }
  }

  return NULL;
}

static bool
is_occupied(struct rl_world const *world, SDL_Point position)
{
  return find_entity(world, position) != NULL;
}

static bool
find_spawn_point(struct rl_world const* world,
                  SDL_Rect const* room,
                  struct rand_state* rng,
                  SDL_Point* out)
{
  int const max_attempts = 10; // hack; temporary?

  for (int attempt = 0; attempt < max_attempts; attempt++) {
    SDL_Point p;
    p.x = (int)rand_next_between(rng, room->x, room->x + room->w - 1);
    p.y = (int)rand_next_between(rng, room->y, room->y + room->h - 1);

    if (!is_occupied(world, p)) {
      *out = p;
      return true;
    }
  }

  return false;
}

static int
max_entities_for_room(SDL_Rect const* room)
{
  int const area = room->w * room->h;

  // scale area down linearly. So, if a rooms area is less than the denominator,
  // scaled becomes 0 (i.e., no enemies in small rooms).
  int const scaled = area / 25;

  return SDL_min(scaled, 4);
}

static void
spawn_entities(struct rl_world *world, struct rand_state *rng)
{
  for (int i = 0; i < world->layout.room_count; i++) {
    SDL_Rect const* room_rect = &world->layout.rooms[i];

    int const count =
      (int)rand_next_up_to(rng, max_entities_for_room(room_rect));
    for (int j = 0; j < count; j++) {
      struct rl_entity entity = rl_create_entity(RL_ENTITY_RAT);

      if(!find_spawn_point(world, room_rect, rng, &entity.position)) {
        continue;
      }

      world->entities[world->entity_count] = entity;
      world->entity_count++;

      if (world->entity_count >= MAX_ENTITIES) {
        return;
      }
    }
  }
}

static bool
try_attack(struct rl_entity* entity,
           struct rl_world const* world,
           SDL_Point dst,
           struct rand_state* rng)
{
  struct rl_entity* e = find_entity(world, dst);
  if (e != NULL) {
    int const damage = rl_attack_entity(entity, e, rng);
    if (damage >= 0) {
      SDL_Log("Attacked %s for %d damage (HP: %d)", e->name, damage, e->hp);
    } else {
      SDL_Log("Missed %s (HP: %d)", e->name, e->hp);
    }

    return true;
  }

  return false;
}

static bool
try_move(struct rl_entity* entity, struct rl_world const* world, SDL_Point dst)
{
  if (rl_is_walkable(rl_get_tile(&world->map, dst.x, dst.y))) {
    entity->position = dst;
    return true;
  }

  return false;
}

static bool
do_move(struct rl_entity* entity,
        struct rl_world const* world,
        SDL_Point direction,
        struct rand_state* rng)
{
  SDL_Point dst = { 0 };
  dst.x = entity->position.x + direction.x;
  dst.y = entity->position.y + direction.y;

  if (try_attack(entity, world, dst, rng)) {
    return true;
  }

  return try_move(entity, world, dst);
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

  // allocate space for the entities
  world->entities = SDL_calloc(MAX_ENTITIES, sizeof(*world->entities));
  if (world->entities == NULL) {
    SDL_Log("SDL_calloc failed: %s", SDL_GetError());
    return false;
  }

  world->entity_count = 0;

  // update the tiles in the map based on the layout
  carve_map(&world->map, &world->layout);

  // the main character
  world->rogue = rl_create_entity(RL_ENTITY_ROGUE);
  // just put the rogue at the centre of the first room
  SDL_Rect const* room = &world->layout.rooms[0];
  world->rogue.position.x = room->x + room->w / 2;
  world->rogue.position.y = room->y + room->h / 2;

  // spawn the other entities
  spawn_entities(world, rng);
  SDL_Log("Number of spawned entities: %d", world->entity_count);

  return true;
}

void
rl_free_world(struct rl_world* world)
{
  if (world == NULL) {
    return;
  }

  SDL_free(world->entities);
  world->entities = NULL;
  world->entity_count = 0;

  rl_free_layout(&world->layout);
  rl_free_map(&world->map);
}

bool
rl_update_world(struct rl_world* world,
                struct rl_command const* cmd,
                struct rand_state* rng)
{
  if (cmd->type == RL_COMMAND_NONE) {
    // if the player didn't take a turn, no other entity should either
    return false;
  }

  bool consume_turn = false;
  switch (cmd->type) {
    case RL_COMMAND_MOVE:
      consume_turn = do_move(&world->rogue, world, cmd->direction, rng);
      break;
    default:
      break;
  }

  // TODO: stop doing a linear scan?
  for(int i = 0; i < world->entity_count; i++) {
    struct rl_entity *e = &world->entities[i];
    if (e->hp <= 0) {
      SDL_Log("%s died", e->name);

      *e = world->entities[world->entity_count - 1];
      world->entity_count--;
    }
  }

  return consume_turn;
}
