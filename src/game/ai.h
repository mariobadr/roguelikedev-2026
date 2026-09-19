/**
 * @file ai.h
 */
#ifndef GINC_ROGUELIKE_AI_H
#define GINC_ROGUELIKE_AI_H

#include "command.h"

// forward declarations
struct rl_actor;
struct rl_fov;
struct rl_world;

bool
rl_wake_actor(struct rl_actor* actor, struct rl_fov const* fov);

struct rl_command
rl_next_ai_command(struct rl_actor const* actor, struct rl_world const* world);

#endif // GINC_ROGUELIKE_AI_H
