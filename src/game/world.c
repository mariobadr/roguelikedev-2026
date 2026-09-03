#include "world.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "procgen/rand.h"

#include "event.h"
#include "fov.h"
#include "pathfinding.h"

static void
carve_map(grid(rl_tile) * map, struct rl_layout const* layout)
{
  enum rl_tile tile = RL_TILE_FLOOR;

  for (int i = 0; i < array_len(&layout->rooms); i++) {
    rl_fill_rect(map, array_at(&layout->rooms, i), tile);
  }

  for (int i = 0; i < array_len(&layout->corridors); i++) {
    struct rl_corridor const* corridor = array_at(&layout->corridors, i);
    for (int j = 0; j < corridor->segment_count; j++) {
      rl_fill_rect(map, &corridor->segments[j], tile);
    }
  }
}

static struct rl_actor*
add_actor(struct rl_world* world, enum rl_actor_type type)
{
  struct rl_actor new_actor = rl_create_actor(type, world->next_actor_id);
  *alist_push(&world->actors) = new_actor;

  return alist_at(&world->actors, world->next_actor_id++);
}

static struct rl_actor*
find_actor(struct rl_world const* world, SDL_Point position)
{
  for (int i = 0; i < alist_len(&world->actors); i++) {
    struct rl_actor* actor = alist_at(&world->actors, i);
    if (actor->pos.x == position.x && actor->pos.y == position.y) {
      return actor;
    }
  }

  return NULL;
}

static bool
is_occupied(struct rl_world const* world, SDL_Point position)
{
  struct rl_actor* actor = find_actor(world, position);

  if (actor == NULL) {
    return false;
  }

  return rl_actor_is_alive(actor);
}

static bool
assign_spawn_point(struct rl_world const* world,
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

  return false; // uh oh
}

static int
max_actors_for_room(SDL_Rect const* room)
{
  int const area = room->w * room->h;

  // scale area down linearly. So, if a rooms area is less than the denominator,
  // scaled becomes 0 (i.e., no enemies in small rooms).
  int const scaled = area / 25;

  return SDL_min(scaled, 4);
}

static void
spawn_actors(struct rl_world* world, struct rand_state* rng)
{
  for (int i = 0; i < array_len(&world->layout.rooms); i++) {
    SDL_Rect const* room_rect = array_at(&world->layout.rooms, i);

    int const count = (int)rand_next_up_to(rng, max_actors_for_room(room_rect));
    for (int j = 0; j < count; j++) {
      struct rl_actor* actor = add_actor(world, RL_ACTOR_RAT);
      assign_spawn_point(world, room_rect, rng, &actor->pos);
    }
  }
}

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

    if (!grid_contains(&world->map, next.x, next.y)) {
      continue;
    }

    int next_distance = *grid_at(&world->distances, next.x, next.y);

    if (next_distance >= best_distance) {
      continue;
    }

    struct rl_actor const* occupant = find_actor(world, next);
    if (occupant != NULL && rl_actor_is_alive(occupant) &&
        occupant->id != RL_ROGUE_ID) {
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
  struct rl_actor* defender = find_actor(world, dst);
  if (defender == NULL || !rl_actor_is_alive(defender)) {
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
  SDL_assert(grid_contains(&world->map, dst.x, dst.y));

  if (rl_is_walkable(*grid_at(&world->map, dst.x, dst.y))) {
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
  // randomly generate the dungeon layout
  if (!rl_init_layout(&world->layout, width, height, rng)) {
    return false;
  }

  // allocate space for the tile map
  if (!grid_alloc(&world->map, width, height)) {
    SDL_Log("grid_alloc failed: %s", SDL_GetError());
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

  // update the tiles in the map based on the layout
  carve_map(&world->map, &world->layout);

  // the main character
  struct rl_actor* rogue = add_actor(world, RL_ACTOR_ROGUE);
  // just put the rogue at the centre of the first room
  SDL_Rect const* room = array_at(&world->layout.rooms, 0);
  rogue->pos.x = room->x + room->w / 2;
  rogue->pos.y = room->y + room->h / 2;

  // spawn the other actors
  spawn_actors(world, rng);
  SDL_Log("Number of spawned actors: %zu", alist_len(&world->actors));

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
  rl_free_layout(&world->layout);
  grid_free(&world->map);
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

  struct rl_actor* rogue = rl_get_actor(world, RL_ROGUE_ID);
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
  if (!rl_build_dijkstra_map(&world->distances, &world->map, rogue->pos)) {
    return;
  }

  // wake up actors in the player's field-of-view and/or
  // move actors closer to the player
  for (int i = 1; i < alist_len(&world->actors); i++) {
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

struct rl_actor*
rl_get_actor(struct rl_world const* world, int id)
{
  if (id >= alist_len(&world->actors)) {
    return NULL;
  }

  return alist_at(&world->actors, id);
}
