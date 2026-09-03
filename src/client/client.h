/**
 * @file client.h
 */
#ifndef GINC_ROGUELIKE_CLIENT_H
#define GINC_ROGUELIKE_CLIENT_H

#include <SDL3/SDL_stdinc.h>

#include "container/alist.h"
#include "game/game_state.h"

#include "client/game_log.h"
#include "client/input.h"
#include "client/resources.h"

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

struct rl_client
{
  /** Transient input state. */
  struct inpt_state istate;
  /** Client resources (like the font). */
  struct rl_resources resources;
  /** Game state. */
  struct rl_game_state game_state;
  struct rl_game_log log;
};

bool
rl_init_client(struct rl_client *client, SDL_Renderer* renderer);

void
rl_free_client(struct rl_client *client);

void
rl_update_client(struct rl_client *client, float dt);

void
rl_render_client(struct rl_client *client, SDL_Renderer* renderer);

#endif // GINC_ROGUELIKE_CLIENT_H
