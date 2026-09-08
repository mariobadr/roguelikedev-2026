#include "ai.h"

#include "actor.h"
#include "fov.h"
#include "pathfinding.h"
#include "world.h"

static bool
pick_direction(SDL_Point* direction,
               struct rl_actor const* actor,
               struct rl_world const* world,
               grid(int) const* distances)
{
  int best_distance = RL_INFINITE_DISTANCE;

  for (size_t i = 0; i < SDL_arraysize(RL_PATH_DIRS); i++) {
    SDL_Point next = { 0 };
    next.x = actor->pos.x + RL_PATH_DIRS[i].x;
    next.y = actor->pos.y + RL_PATH_DIRS[i].y;

    if (!grid_contains(&world->level.map, next.x, next.y)) {
      continue;
    }

    int next_distance = *grid_at(distances, next.x, next.y);

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

bool
rl_wake_actor(struct rl_actor* actor, struct rl_fov const* fov)
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
  return true;
}

struct rl_command
rl_next_ai_command(struct rl_actor const* actor,
                   struct rl_world const* world,
                   grid(int) const* distances)
{
  SDL_Point dir;
  if (pick_direction(&dir, actor, world, distances)) {
    return rl_new_bump_command(actor->id, dir, world);
  }

  return (struct rl_command){
    .actor = actor->id,
    .type = RL_COMMAND_NONE,
  };
}
