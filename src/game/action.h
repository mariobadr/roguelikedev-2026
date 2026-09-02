/**
 * @file action.h
 */
#ifndef GINC_ROGUELIKE_ACTION_H
#define GINC_ROGUELIKE_ACTION_H

/**
 * How long between subsequent actions.
 */
#define ACTION_GLOBAL_COOLDOWN (0.115f)

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
  RL_ACTION_DEBUG_GENMAP
};

#endif // GINC_ROGUELIKE_ACTION_H
