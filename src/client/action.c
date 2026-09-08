#include "action.h"

struct rl_command
rl_build_command(int actor_id,
                 enum rl_action action,
                 struct rl_world const* world)
{
  struct rl_command cmd = { 0 };

  switch (action) {
    case RL_ACTION_MOVE_UP:
      return rl_new_bump_command(actor_id, (SDL_Point){ 0, -1 }, world);
    case RL_ACTION_MOVE_DOWN:
      return rl_new_bump_command(actor_id, (SDL_Point){ 0, 1 }, world);
    case RL_ACTION_MOVE_LEFT:
      return rl_new_bump_command(actor_id, (SDL_Point){ -1, 0 }, world);
    case RL_ACTION_MOVE_RIGHT:
      return rl_new_bump_command(actor_id, (SDL_Point){ 1, 0 }, world);
    case RL_ACTION_INTERACT:
      break; // TODO
    default:
      break;
  }

  cmd.actor = actor_id;
  return cmd;
}
