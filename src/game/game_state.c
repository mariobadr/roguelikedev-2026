#include "game_state.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "command.h"

#define FOV_RADIUS 8

/**
 * TODO: fix all the duplication between here and rl_init_game_state
 */
static bool
regenerate_map(struct rl_game_state* game_state)
{
  struct rl_world new_world = { 0 };
  if (!rl_init_world(
        &new_world,
        game_state->map_width,
        game_state->map_height,
        &game_state->rng)) {
    return false;
  }

  rl_free_world(&game_state->world);
  game_state->world = new_world;

  rl_clear_fov(&game_state->fov);

  struct rl_entity const* rogue =
    rl_get_entity(&game_state->world, RL_ROGUE_ID);
  rl_update_fov(&game_state->fov, &game_state->world.map, rogue->position);

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

  if (!rl_init_fov(&game_state->fov, map_width * map_height, FOV_RADIUS)) {
    rl_free_game_state(game_state);
    return false;
  }

  // make sure the rogue has an initial field-of-view
  struct rl_entity const* rogue =
    rl_get_entity(&game_state->world, RL_ROGUE_ID);
  rl_update_fov(&game_state->fov, &game_state->world.map, rogue->position);

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

  game_state->action_cooldown =
    SDL_max(0.0f, game_state->action_cooldown - dt);
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
  struct rl_command cmd = rl_build_command(action);

  // Do the simulation
  if (rl_update_world(
        &game_state->world, &cmd, &game_state->events, &game_state->rng)) {
    // turn taken
    game_state->action_cooldown = ACTION_GLOBAL_COOLDOWN;
  }

  struct rl_entity const* rogue =
    rl_get_entity(&game_state->world, RL_ROGUE_ID);
  for (int i = 0; i < alist_len(&game_state->events); i++) {
    struct rl_event const* event = alist_at(&game_state->events, i);

    switch (event->type) {
      case RL_EVENT_MOVE:
        rl_update_fov(
          &game_state->fov, &game_state->world.map, rogue->position);
        break;
      default:
        break;
    }
  }
}
