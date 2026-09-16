#include "game.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "actor.h"
#include "actor_def.h"
#include "ai.h"
#include "command.h"
#include "generate.h"
#include "pathfinding.h"

#define FOV_RADIUS 8

static void
update_actors(struct rl_world* world,
              grid(int) * distances,
              struct rl_fov const* fov,
              alist(rl_event) * events,
              struct rand_state* rng)
{
  handle(rl_actor) const rogue_handle = rl_get_rogue(world);
  struct rl_actor const* rogue = rl_borrow_actor(world, rogue_handle);
  if (!rl_actor_is_alive(rogue)) {
    // the player is dead
    return;
  }

  // build the distance map where the target is the player
  struct rl_level const* level = rl_get_current_level(world);
  if (!rl_build_dijkstra_map(distances, &level->map, rogue->pos)) {
    return;
  }

  // wake up actors in the player's field-of-view and/or
  // move actors closer to the player
  for (size_t i = 0; i < alist_len(&level->actors); i++) {
    handle(rl_actor) const actor_handle = *alist_at(&level->actors, i);
    struct rl_actor* actor = rl_borrow_mut_actor(world, actor_handle);
    if (actor == NULL) {
      continue;
    }

    if (handle_equal(actor->handle, rogue_handle)) {
      // the player is not controlled by the AI
      continue;
    }

    if (!rl_actor_is_alive(actor)) {
      // actor is dead
      continue;
    }

    bool const was_awake = actor->awake;
    if (!rl_wake_actor(actor, fov)) {
      // actor is asleep
      continue;
    }

    if (!was_awake) {
      struct rl_event event = { 0 };
      event.type = RL_EVENT_AWAKEN;
      event.as.awaken.actor = actor->handle;
      *alist_push(events) = event;
    }

    // choose and execute each actor's command before updating the next actor
    struct rl_command cmd = rl_next_ai_command(actor, world, distances);
    rl_apply_command(world, &cmd, fov, events, rng);

    // the rogue lives in world->actors, which may have been reallocated
    rogue = rl_borrow_actor(world, rogue_handle);
    if (!rl_actor_is_alive(rogue)) {
      // the player is dead
      return;
    }
  }
}

static void
update_explored(struct rl_level* level, struct rl_fov const* fov)
{
  for (size_t i = 0; i < grid_count(&fov->visible); i++) {
    if (*grid_at_index(&fov->visible, i)) {
      *grid_at_index(&level->explored, i) = true;
    }
  }
}

static bool
alloc_map_buffers(struct rl_game* game, int width, int height)
{
  // allocate space for the distance map
  if (!grid_alloc(&game->distances, width, height)) {
    SDL_Log("grid_alloc failed: %s", SDL_GetError());
    return false;
  }

  if (!rl_alloc_fov(&game->fov, width, height, FOV_RADIUS)) {
    return false;
  }

  return true;
}

bool
rl_alloc_game(struct rl_game* game)
{
  if (!rl_alloc_world(&game->world)) {
    return false;
  }

  return true;
}

bool
rl_new_game(struct rl_game* game, int width, int height, Uint64 seed)
{
  rand_seed(&game->rng, seed);

  // create the main character
  handle(rl_actor) rogue_handle =
    rl_create_actor(&game->world, RL_ACTOR_ROGUE);
  struct rl_actor* rogue_slot = rl_borrow_mut_actor(&game->world, rogue_handle);
  if (rogue_slot == NULL) {
    return false;
  }
  rogue_slot->awake = true;
  game->world.rogue = rogue_slot->handle;

  struct rl_level* level = alist_push(&game->world.levels);
  if (level == NULL) {
    return false;
  }

  if (!rl_alloc_level(level, 1, width, height)) {
    return false;
  }
  game->world.current_level = (int)alist_len(&game->world.levels) - 1;

  // the rogue takes the first turn on its level
  if (!rl_add_actor(level, game->world.rogue)) {
    return false;
  }

  if (!rl_gen_level(&game->world, level, &game->rng)) {
    return false;
  }

  if (!alloc_map_buffers(game, width, height)) {
    return false;
  }

  // make sure the rogue has an initial field-of-view
  struct rl_actor const* rogue =
    rl_borrow_actor(&game->world, rl_get_rogue(&game->world));
  rl_update_fov(&game->fov, &level->map, rogue->pos);
  update_explored(level, &game->fov);

  return true;
}

void
rl_free_game(struct rl_game* game)
{
  if (game == NULL) {
    return;
  }

  rl_free_fov(&game->fov);
  grid_free(&game->distances);
  rl_free_world(&game->world);
}

bool
rl_update_game(struct rl_game* game,
               struct rl_command const* cmd,
               alist(rl_event) * events)
{
  bool turn_taken =
    rl_apply_command(&game->world, cmd, &game->fov, events, &game->rng);

  if (turn_taken) {
    struct rl_actor const* rogue =
      rl_borrow_actor(&game->world, rl_get_rogue(&game->world));

    struct rl_level* level = rl_edit_current_level(&game->world);
    rl_update_fov(&game->fov, &level->map, rogue->pos);
    update_explored(level, &game->fov);

    update_actors(
      &game->world, &game->distances, &game->fov, events, &game->rng);
  }

  return turn_taken;
}
