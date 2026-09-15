#include "action.h"

#include <SDL3/SDL_assert.h>

#include "game/actor.h"

static struct rl_command
build_pickup(struct rl_actor const* actor)
{
  struct rl_command cmd = { 0 };

  if (actor == NULL) {
    return cmd;
  }

  cmd.type = RL_COMMAND_PICK_UP;
  cmd.actor = actor->handle;
  cmd.dst = actor->pos;

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
      return build_pickup(actor);
    case RL_ACTION_WAIT:
      cmd.type = RL_COMMAND_WAIT;
      break;
    default:
      break;
  }

  cmd.actor = actor->handle;
  return cmd;
}
