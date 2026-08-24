#include "command.h"

struct rl_command
create_move_command(int dx, int dy)
{
  struct rl_command cmd = { 0 };

  cmd.type = RL_COMMAND_MOVE;
  cmd.direction.x = dx;
  cmd.direction.y = dy;

  return cmd;
}

struct rl_command
rl_build_command(enum rl_action action)
{
  switch (action) {
    case RL_ACTION_MOVE_UP:
      return create_move_command(0, -1);
    case RL_ACTION_MOVE_DOWN:
      return create_move_command(0, 1);
    case RL_ACTION_MOVE_LEFT:
      return create_move_command(-1, 0);
    case RL_ACTION_MOVE_RIGHT:
      return create_move_command(1, 0);
    default:
      break;
  }

  struct rl_command cmd = { 0 };
  return cmd;
}
