#include "ai.h"

#include "spatial/fov.h"

#include "actor.h"
#include "movement.h"
#include "world.h"

static bool
pick_direction(SDL_Point* direction,
               struct rl_actor const* actor,
               struct rl_world const* world)
{
  int best_distance = SPTL_UNREACHABLE;

  handle(rl_actor) const rogue = rl_get_rogue(world);
  struct rl_level const* level = rl_get_current_level(world);
  for (size_t i = 0; i < SDL_arraysize(RL_STEP_DIRS); i++) {
    SDL_Point next = { 0 };
    next.x = actor->pos.x + RL_STEP_DIRS[i].x;
    next.y = actor->pos.y + RL_STEP_DIRS[i].y;

    int next_distance =
      *grid_at(&world->player.scent.distances, next.x, next.y);

    if (next_distance >= best_distance) {
      continue;
    }

    handle(rl_actor) const occupant = rl_find_actor(world, level, next);
    if (handle_is_nonnull(occupant) && !handle_equal(occupant, rogue)) {
      // the tile is occupied by a non-rogue actor
      continue;
    }

    best_distance = next_distance;
    *direction = RL_STEP_DIRS[i];
  }

  if (best_distance == SPTL_UNREACHABLE) {
    return false;
  }

  return true;
}

bool
rl_wake_actor(struct rl_actor* actor, struct sptl_fov const* fov)
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
rl_next_ai_command(struct rl_actor const* actor, struct rl_world const* world)
{
  SDL_Point dir;
  if (pick_direction(&dir, actor, world)) {
    return rl_new_bump_command(actor, dir, world);
  }

  return (struct rl_command){
    .actor = actor->handle,
    .type = RL_COMMAND_NONE,
  };
}
