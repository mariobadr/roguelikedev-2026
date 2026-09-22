/**
 * @file ai.h
 */
#ifndef GINC_ROGUELIKE_AI_H
#define GINC_ROGUELIKE_AI_H

#include "command.h"

// forward declarations
struct rl_actor;
struct rl_world;

// external forward declarations
struct sptl_fov;

/**
 * Potentially wake actor.
 *
 * @return whether actor is awake.
 */
bool
rl_wake_actor(struct rl_actor* actor, struct sptl_fov const* fov);

/**
 * @return a command based on the world.
 */
struct rl_command
rl_next_ai_command(struct rl_actor const* actor, struct rl_world const* world);

#endif // GINC_ROGUELIKE_AI_H
