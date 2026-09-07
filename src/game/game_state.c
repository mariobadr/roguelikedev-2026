#include "game_state.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "ai.h"
#include "command.h"
#include "pathfinding.h"

#define FOV_RADIUS 8

static void
update_actors(struct rl_world* world,
              grid(int) * distances,
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
  if (!rl_build_dijkstra_map(distances, &world->level.map, rogue->pos)) {
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

    bool const was_awake = actor->awake;
    if (!rl_wake_actor(actor, fov)) {
      // actor is asleep
      continue;
    }

    if (!was_awake) {
      struct rl_event event = { 0 };
      event.type = RL_EVENT_AWAKEN;
      event.as.awaken.actor = actor->id;
      *alist_push(events) = event;
    }

    // choose and execute each actor's command before updating the next actor
    struct rl_command cmd = rl_next_ai_command(actor, world, distances);
    rl_apply_command(world, &cmd, events, rng);

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

/**
 * TODO: fix all the duplication between here and rl_init_game_state
 */
static bool
regenerate_map(struct rl_game_state* game_state)
{
  struct rl_world new_world = { 0 };
  if (!rl_init_world(&new_world,
                     game_state->map_width,
                     game_state->map_height,
                     &game_state->rng)) {
    return false;
  }

  rl_free_world(&game_state->world);
  game_state->world = new_world;

  rl_clear_fov(&game_state->fov);

  struct rl_actor const* rogue = rl_get_actor(&game_state->world, RL_ROGUE_ID);
  rl_update_fov(&game_state->fov, &game_state->world.level.map, rogue->pos);
  update_explored(&game_state->world.level, &game_state->fov);

  return true;
}

bool
rl_init_game_state(struct rl_game_state* game_state,
                   int map_width,
                   int map_height)
{
  rand_seed(&game_state->rng, 1234);

  if (!alist_alloc(&game_state->events, 8)) {
    SDL_Log("alist_alloc failed: %s", SDL_GetError());
    return false;
  }

  if (!rl_init_world(
        &game_state->world, map_width, map_height, &game_state->rng)) {
    rl_free_game_state(game_state);
    return false;
  }

  // allocate space for the distance map
  if (!grid_alloc(&game_state->distances, map_width, map_height)) {
    SDL_Log("grid_alloc failed: %s", SDL_GetError());
    rl_free_game_state(game_state);
    return false;
  }

  if (!rl_init_fov(&game_state->fov, map_width, map_height, FOV_RADIUS)) {
    rl_free_game_state(game_state);
    return false;
  }

  // make sure the rogue has an initial field-of-view
  struct rl_actor const* rogue = rl_get_actor(&game_state->world, RL_ROGUE_ID);
  rl_update_fov(&game_state->fov, &game_state->world.level.map, rogue->pos);
  update_explored(&game_state->world.level, &game_state->fov);

  game_state->map_width = map_width;
  game_state->map_height = map_height;
  game_state->action_cooldown = 0.0f;

  return true;
}

void
rl_free_game_state(struct rl_game_state* game_state)
{
  if (game_state == NULL) {
    return;
  }

  rl_free_fov(&game_state->fov);
  grid_free(&game_state->distances);
  rl_free_world(&game_state->world);
  alist_free(&game_state->events);
}

void
rl_update_game_state(struct rl_game_state* game_state,
                     enum rl_action action,
                     float dt)
{
  // clear the last update's events
  alist_clear(&game_state->events);

  game_state->action_cooldown = SDL_max(0.0f, game_state->action_cooldown - dt);
  if (game_state->action_cooldown > 0.0f) {
    return;
  }

  if (action == RL_ACTION_NONE) {
    return;
  }

  if (action == RL_ACTION_DEBUG_GENMAP) {
    if (regenerate_map(game_state)) {
      game_state->action_cooldown = ACTION_GLOBAL_COOLDOWN;
    }

    return;
  }

  // Build a command based on the player's last action
  struct rl_command cmd = rl_build_command(RL_ROGUE_ID, action);

  bool turn_taken = rl_apply_command(
    &game_state->world, &cmd, &game_state->events, &game_state->rng);

  if (turn_taken) {
    game_state->action_cooldown = ACTION_GLOBAL_COOLDOWN;

    struct rl_actor const* rogue =
      rl_get_actor(&game_state->world, RL_ROGUE_ID);
    rl_update_fov(&game_state->fov, &game_state->world.level.map, rogue->pos);
    update_explored(&game_state->world.level, &game_state->fov);
    update_actors(&game_state->world,
                  &game_state->distances,
                  &game_state->fov,
                  &game_state->events,
                  &game_state->rng);
  }
}
