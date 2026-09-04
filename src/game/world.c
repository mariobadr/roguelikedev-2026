#include "world.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "procgen/rand.h"

#include "combat.h"
#include "event.h"
#include "fov.h"
#include "pathfinding.h"
#include "spawn.h"

static bool
steer_actor(SDL_Point* direction,
            struct rl_actor const* actor,
            struct rl_world const* world)
{
  int best_distance = RL_INFINITE_DISTANCE;

  for (size_t i = 0; i < SDL_arraysize(RL_PATH_DIRS); i++) {
    SDL_Point next = { 0 };
    next.x = actor->pos.x + RL_PATH_DIRS[i].x;
    next.y = actor->pos.y + RL_PATH_DIRS[i].y;

    if (!grid_contains(&world->level.map, next.x, next.y)) {
      continue;
    }

    int next_distance = *grid_at(&world->distances, next.x, next.y);

    if (next_distance >= best_distance) {
      continue;
    }

    struct rl_actor const* occupant = rl_find_actor(world, next);
    if (occupant != NULL && occupant->id != RL_ROGUE_ID) {
      // the tile is occupied by a non-rogue actor
      continue;
    }

    best_distance = next_distance;
    *direction = RL_PATH_DIRS[i];
  }

  if (best_distance == RL_INFINITE_DISTANCE) {
    return false;
  }

  return true;
}

static bool
try_wake(struct rl_actor* actor,
         struct rl_fov const* fov,
         alist(rl_event) * events)
{
  if (actor->awake) {
    // already awake
    return true;
  }

  if (!*grid_at(&fov->visible, actor->pos.x, actor->pos.y)) {
    // actor hasn't seen player yet
    return false;
  }

  actor->awake = true;

  struct rl_event event = { 0 };
  event.type = RL_EVENT_AWAKEN;
  event.as.awaken.actor = actor->id;
  *alist_push(events) = event;

  return true;
}

static bool
try_attack(struct rl_actor* attacker,
           struct rl_world const* world,
           SDL_Point dst,
           alist(rl_event) * events,
           struct rand_state* rng)
{
  struct rl_actor* defender = rl_find_actor(world, dst);
  if (defender == NULL) {
    return false;
  }

  int const damage = rl_attack_actor(attacker, defender, rng);

  struct rl_event event = { 0 };
  event.type = RL_EVENT_ATTACK;
  event.as.attack.attacker = attacker->id;
  event.as.attack.defender = defender->id;
  event.as.attack.damage = damage;
  *alist_push(events) = event;

  if (defender->hp <= 0) {
    event.type = RL_EVENT_DEATH;
    event.as.death.actor = defender->id;
    event.as.death.killer = attacker->id;
    *alist_push(events) = event;
  }

  return true;
}

static bool
try_move(struct rl_actor* actor, struct rl_world const* world, SDL_Point dst)
{
  SDL_assert(grid_contains(&world->level.map, dst.x, dst.y));

  if (rl_is_walkable(*grid_at(&world->level.map, dst.x, dst.y))) {
    actor->pos = dst;
    return true;
  }

  return false;
}

static bool
do_move(struct rl_actor* actor,
        struct rl_world const* world,
        SDL_Point direction,
        alist(rl_event) * events,
        struct rand_state* rng)
{
  SDL_Point dst = { 0 };
  dst.x = actor->pos.x + direction.x;
  dst.y = actor->pos.y + direction.y;

  if (try_attack(actor, world, dst, events, rng)) {
    return true;
  }

  if (try_move(actor, world, dst)) {
    return true;
  }

  return false;
}

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

  // allocate space for the distance map
  if (!grid_alloc(&world->distances, width, height)) {
    SDL_Log("grid_alloc failed: %s", SDL_GetError());
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

  return true;
}

void
rl_free_world(struct rl_world* world)
{
  if (world == NULL) {
    return;
  }

  grid_free(&world->distances);
  alist_free(&world->actors);
  rl_free_level(&world->level);
}

struct rl_actor*
rl_get_actor(struct rl_world const* world, int id)
{
  if (id == RL_ROGUE_ID) {
    return (struct rl_actor*)&world->rogue;
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

struct rl_item*
rl_get_item(struct rl_world const* world, int id)
{
  if (id >= alist_len(&world->items)) {
    return NULL;
  }

  return alist_at(&world->items, id);
}

struct rl_actor*
rl_find_actor(struct rl_world const* world, SDL_Point position)
{
  for (int id = 0; id < rl_actor_count(world); id++) {
    struct rl_actor* actor = rl_get_actor(world, id);
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

bool
rl_apply_command(struct rl_world* world,
                 struct rl_command const* cmd,
                 alist(rl_event) * events,
                 struct rand_state* rng)
{
  if (cmd->type == RL_COMMAND_NONE) {
    return false;
  }

  struct rl_actor* rogue = &world->rogue;
  if (!rl_actor_is_alive(rogue)) {
    // the rogue is dead
    return false;
  }

  bool consume_turn = false;
  switch (cmd->type) {
    case RL_COMMAND_MOVE:
      consume_turn = do_move(rogue, world, cmd->direction, events, rng);
      break;
    default:
      break;
  }

  return consume_turn;
}

void
rl_update_actors(struct rl_world* world,
                 struct rl_fov const* fov,
                 alist(rl_event) * events,
                 struct rand_state* rng)
{
  struct rl_actor const* rogue = rl_get_actor(world, RL_ROGUE_ID);
  if (!rl_actor_is_alive(rogue)) {
    // the player is dead
    return;
  }

  // build the distance map where the target is the player
  if (!rl_build_dijkstra_map(&world->distances, &world->level.map, rogue->pos)) {
    return;
  }

  // wake up actors in the player's field-of-view and/or
  // move actors closer to the player
  for (int i = 0; i < alist_len(&world->actors); i++) {
    struct rl_actor* actor = alist_at(&world->actors, i);

    if (!rl_actor_is_alive(actor)) {
      // actor is dead
      continue;
    }

    if (!try_wake(actor, fov, events)) {
      // actor is asleep
      continue;
    }

    // move the actor toward the player
    SDL_Point direction;
    if (steer_actor(&direction, actor, world)) {
      do_move(actor, world, direction, events, rng);
    }

    if (!rl_actor_is_alive(rogue)) {
      // the player is dead
      return;
    }
  }
}
