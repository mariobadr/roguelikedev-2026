#include "world.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "spawn.h"

bool
rl_init_world(struct rl_world* world,
              int width,
              int height,
              struct rand_state* rng)

{
  // allocate space for the level (map + explored grids)
  if (!rl_alloc_level(&world->level, 1, width, height)) {
    rl_free_world(world);
    return false;
  }

  // allocate space for the actors
  if (!alist_alloc(&world->actors, 16)) {
    SDL_Log("alist_alloc failed: %s", SDL_GetError());
    rl_free_world(world);
    return false;
  }

  // allocate space for the items
  if (!alist_alloc(&world->items, 8)) {
    SDL_Log("alist_alloc failed: %s", SDL_GetError());
    rl_free_world(world);
    return false;
  }

  // randomly generate the dungeon layout and carve it into the map
  if (!rl_gen_level(&world->level, rng)) {
    rl_free_world(world);
    return false;
  }

  // the main character
  world->rogue = rl_create_actor(RL_ACTOR_ROGUE, RL_ROGUE_ID);
  // just put the rogue at the centre of the first room
  int const rogue_room = 0;
  SDL_Rect const* room = array_at(&world->level.layout.rooms, rogue_room);
  world->rogue.pos.x = room->x + room->w / 2;
  world->rogue.pos.y = room->y + room->h / 2;

  // spawn the other actors
  if (!rl_spawn_actors(&world->level, &world->actors, rogue_room, rng)) {
    rl_free_world(world);
    return false;
  }
  SDL_Log("Number of spawned actors: %d", rl_actor_count(world));

  // spawn items
  if (!rl_spawn_items(&world->level, &world->items, rng)) {
    rl_free_world(world);
    return false;
  }
  SDL_Log("Number of spawned items: %d", (int)alist_len(&world->items));

  return true;
}

void
rl_free_world(struct rl_world* world)
{
  if (world == NULL) {
    return;
  }

  alist_free(&world->items);
  alist_free(&world->actors);
  rl_free_level(&world->level);
}

struct rl_actor const*
rl_get_actor(struct rl_world const* world, int id)
{
  if (id == RL_ROGUE_ID) {
    return &world->rogue;
  }

  int const index = id - 1;
  if (index < 0 || index >= alist_len(&world->actors)) {
    return NULL;
  }

  return alist_at(&world->actors, index);
}

struct rl_actor*
rl_edit_actor(struct rl_world* world, int id)
{
  if (id == RL_ROGUE_ID) {
    return &world->rogue;
  }

  int const index = id - 1;
  if (index < 0 || index >= alist_len(&world->actors)) {
    return NULL;
  }

  return alist_at(&world->actors, index);
}

int
rl_actor_count(struct rl_world const* world)
{
  return 1 + (int)alist_len(&world->actors);
}

struct rl_item const*
rl_get_item(struct rl_world const* world, int id)
{
  if (id >= alist_len(&world->items)) {
    return NULL;
  }

  return alist_at(&world->items, id);
}

struct rl_actor const*
rl_find_actor(struct rl_world const* world, SDL_Point position)
{
  for (int id = 0; id < rl_actor_count(world); id++) {
    struct rl_actor const* actor = rl_get_actor(world, id);
    if (!rl_actor_is_alive(actor)) {
      // ignore dead actors
      continue;
    }

    if (actor->pos.x == position.x && actor->pos.y == position.y) {
      return actor;
    }
  }

  return NULL;
}
