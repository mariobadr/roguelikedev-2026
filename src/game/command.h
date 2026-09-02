/**
 * @file command.h
 */
#ifndef GINC_ROGUELIKE_COMMAND_H
#define GINC_ROGUELIKE_COMMAND_H

#include <SDL3/SDL_rect.h>

#include "action.h"

/**
 * The actions a player can perform
 */
enum rl_command_type
{
  RL_COMMAND_NONE, //< No action
  RL_COMMAND_MOVE, //< Move the player
};

/**
 * An action that can be applied to update the game.
 */
struct rl_command
{
  enum rl_command_type type;
  union
  {
    SDL_Point direction;
  };
};

struct rl_command
rl_build_command(enum rl_action action);

#endif // GINC_ROGUELIKE_COMMAND_H
