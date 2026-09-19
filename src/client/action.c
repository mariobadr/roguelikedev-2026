#include "action.h"

#include <SDL3/SDL_assert.h>

#include "game/actor.h"
#include "game/mechanics.h"
#include "game/world.h"

enum rl_interaction
rl_available_interaction(struct rl_actor const* actor,
                          struct rl_world const* world)
{
  if (actor == NULL || !rl_actor_is_alive(actor)) {
    return RL_INTERACTION_NONE;
  }

  struct rl_level const* level = rl_get_current_level(world);
  if (handle_is_nonnull(rl_find_item(world, level, actor->pos))) {
    return RL_INTERACTION_PICK_UP;
  }

  if (rl_can_take_stairs(world, actor)) {
    return RL_INTERACTION_TAKE_STAIRS;
  }

  return RL_INTERACTION_NONE;
}

static struct rl_command
build_interact(struct rl_actor const* actor, struct rl_world const* world)
{
  struct rl_command cmd = { 0 };

  switch (rl_available_interaction(actor, world)) {
    case RL_INTERACTION_PICK_UP:
      cmd.type = RL_COMMAND_PICK_UP;
      cmd.actor = actor->handle;
      cmd.dst = actor->pos;
      break;
    case RL_INTERACTION_TAKE_STAIRS:
      cmd.type = RL_COMMAND_TAKE_STAIRS;
      cmd.actor = actor->handle;
      break;
    case RL_INTERACTION_NONE:
      break;
  }

  return cmd;
}

struct rl_command
rl_build_command(struct rl_actor const* actor,
                 enum rl_action action,
                 struct rl_world const* world)
{
  struct rl_command cmd = { 0 };

  if (actor == NULL) {
    return cmd;
  }

  switch (action) {
    case RL_ACTION_MOVE_UP:
      return rl_new_bump_command(actor, (SDL_Point){ 0, -1 }, world);
    case RL_ACTION_MOVE_DOWN:
      return rl_new_bump_command(actor, (SDL_Point){ 0, 1 }, world);
    case RL_ACTION_MOVE_LEFT:
      return rl_new_bump_command(actor, (SDL_Point){ -1, 0 }, world);
    case RL_ACTION_MOVE_RIGHT:
      return rl_new_bump_command(actor, (SDL_Point){ 1, 0 }, world);
    case RL_ACTION_SELECT:
      return build_interact(actor, world);
    case RL_ACTION_WAIT:
      cmd.type = RL_COMMAND_WAIT;
      break;
    default:
      break;
  }

  cmd.actor = actor->handle;
  return cmd;
}
