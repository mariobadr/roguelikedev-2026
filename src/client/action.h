/**
 * @file action.h
 */
#ifndef GINC_ROGUELIKE_ACTION_H
#define GINC_ROGUELIKE_ACTION_H

#include "game/command.h"

/**
 * The actions a player can perform
 */
enum rl_action
{
  RL_ACTION_NONE,
  RL_ACTION_MOVE_UP,
  RL_ACTION_MOVE_DOWN,
  RL_ACTION_MOVE_LEFT,
  RL_ACTION_MOVE_RIGHT,
  RL_ACTION_SELECT,
  RL_ACTION_WAIT,
  RL_ACTION_TOGGLE_INVENTORY,
  RL_ACTION_FOCUS_NEXT,
};

/**
 * @return a command that corresponds to the given action.
 */
struct rl_command
rl_build_command(int actor_id,
                 enum rl_action action,
                 struct rl_world const* world);

#endif // GINC_ROGUELIKE_ACTION_H
