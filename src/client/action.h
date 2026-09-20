/**
 * @file action.h
 */
#ifndef GINC_ROGUELIKE_ACTION_H
#define GINC_ROGUELIKE_ACTION_H

#include "game/command.h"

/**
 * The actions a player can perform.
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
  RL_ACTION_SHOW_INVENTORY,
  RL_ACTION_SHOW_LOG,
  RL_ACTION_CANCEL,
};

/**
 * The interactions an actor can have with its surroundings.
 */
enum rl_interaction
{
  RL_INTERACTION_NONE,
  RL_INTERACTION_PICK_UP,
  RL_INTERACTION_TAKE_STAIRS,
};

/**
 * @return the available interaction for actor.
 */
enum rl_interaction
rl_available_interaction(struct rl_actor const* actor,
                         struct rl_world const* world);

/**
 * If actor is NULL or the action has no effect here, then the NONE command is
 * returned.
 *
 * @return a command that corresponds to the given action.
 */
struct rl_command
rl_build_command(struct rl_actor const* actor,
                 enum rl_action action,
                 struct rl_world const* world);

#endif // GINC_ROGUELIKE_ACTION_H
