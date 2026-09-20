/**
 * @file command.h
 */
#ifndef GINC_ROGUELIKE_COMMAND_H
#define GINC_ROGUELIKE_COMMAND_H

#include <SDL3/SDL_rect.h>

#include "game/event.h"
#include "game/handles.h"

// forward declarations
struct rl_actor;
struct rl_world;
struct rand_state;

/**
 * The types of commands an actor can apply.
 */
enum rl_command_type
{
  RL_COMMAND_NONE,        //< no action
  RL_COMMAND_MOVE,        //< move the actor
  RL_COMMAND_ATTACK,      //< attack an actor
  RL_COMMAND_PICK_UP,     //< pick up an item
  RL_COMMAND_USE_ITEM,    //< use an item
  RL_COMMAND_TAKE_STAIRS, //< take the stairs
  RL_COMMAND_WAIT,        //< consume a turn without acting
};

/**
 * An item to use and where to use it.
 */
struct rl_item_use
{
  /** The item being used. */
  handle(rl_item) item;
  /** The target location, in tile coordinates. */
  SDL_Point dst;
};

/**
 * A command that can be applied to the game.
 */
struct rl_command
{
  /** The actor performing this command. */
  handle(rl_actor) actor;
  /** The kind of command. */
  enum rl_command_type type;

  union
  {
    /** MOVE, PICK UP */
    SDL_Point dst;
    /** ATTACK */
    handle(rl_actor) target_actor;
    /** USE ITEM */
    struct rl_item_use use_item;
  };
};

/**
 * Borrow actor from world for this call; the command stores actor handles.
 *
 * @return a move or attack command.
 * Returns RL_COMMAND_NONE if actor is NULL, dead, or cannot move or attack.
 */
struct rl_command
rl_new_bump_command(struct rl_actor const* actor,
                    SDL_Point dir,
                    struct rl_world const* world);

/**
 * Apply cmd to world, recording what happens in events.
 *
 * @return whether applying the command consumes a turn.
 */
bool
rl_apply_command(struct rl_world* world,
                 struct rl_command const* cmd,
                 alist(rl_event)* events,
                 struct rand_state* rng);

#endif // GINC_ROGUELIKE_COMMAND_H
