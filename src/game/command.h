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
struct rl_fov;
struct rand_state;

/**
 * The types of commands an actor can apply.
 */
enum rl_command_type
{
  RL_COMMAND_NONE,     //< No action
  RL_COMMAND_MOVE,     //< Move the actor
  RL_COMMAND_ATTACK,   //< Attack an actor
  RL_COMMAND_PICK_UP,  //< Pick up an item
  RL_COMMAND_USE_ITEM, //< Use an item
  RL_COMMAND_WAIT,     //< Consume a turn without acting
};

struct rl_item_use
{
  handle(rl_item) item;
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
 * Borrows actor from world for this call; the command stores actor handles.
 *
 * @return a move or attack command.
 * Returns RL_COMMAND_NONE if actor is NULL, dead, or cannot move or attack.
 */
struct rl_command
rl_new_bump_command(struct rl_actor const* actor,
                    SDL_Point dir,
                    struct rl_world const* world);

/**
 * @return whether applying the command consumes a turn.
 */
bool
rl_apply_command(struct rl_world* world,
                 struct rl_command const* cmd,
                 struct rl_fov const* fov,
                 alist(rl_event)* events,
                 struct rand_state* rng);

#endif // GINC_ROGUELIKE_COMMAND_H
