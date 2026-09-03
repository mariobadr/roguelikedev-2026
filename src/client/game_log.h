/**
 * @file game_log.h
 */
#ifndef GINC_ROGUELIKE_GAME_LOG_H
#define GINC_ROGUELIKE_GAME_LOG_H

#include "container/alist.h"

#include "client/text.h"

// forward declarations
struct rl_event;
struct rl_world;

alist_define_as(struct rl_text, rl_message);

struct rl_game_log
{
  alist(rl_message) messages;
};

bool
rl_init_game_log(struct rl_game_log* log);

void
rl_free_game_log(struct rl_game_log* log);

void
rl_game_log_on_event(struct rl_game_log* log,
                     struct rl_event const* event,
                     struct rl_world const* world);

#endif // GINC_ROGUELIKE_GAME_LOG_H
