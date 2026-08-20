/**
 * @file action.h
 */
#ifndef GINC_ROGUELIKE_ACTION_H
#define GINC_ROGUELIKE_ACTION_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#define ACTION_GLOBAL_COOLDOWN (0.115f)

/**
 * The actions a player can perform
 */
enum rl_action_type
{
  RL_ACTION_NONE, //< No action
  RL_ACTION_MOVE, //< Move the player
};

/**
 * An action that can be applied to update the game.
 */
struct rl_action
{
  enum rl_action_type type;
  union
  {
    SDL_Point move_vector;
  };
};

#endif // GINC_ROGUELIKE_ACTION_H
