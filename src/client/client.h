/**
 * @file client.h
 */
#ifndef GINC_ROGUELIKE_CLIENT_H
#define GINC_ROGUELIKE_CLIENT_H

#include <SDL3/SDL_stdinc.h>

#include "container/array.h"

#include "client/font.h"
#include "client/screen.h"

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct inpt_state;

array_define_as(enum rl_screen_id, rl_screen_id);

struct rl_client
{
  /** Bitmap font used to draw everything. */
  struct rl_font font;
  /** Allocated screens, retained when removed from the stack. */
  struct rl_screen screens[RL_SCREEN_COUNT];
  /** Each screen may appear at most once; only the top is active. */
  array(rl_screen_id) stack;
};

/** Initialize a zero-initialized client. */
bool
rl_init_client(struct rl_client* client, SDL_Renderer* renderer);

void
rl_free_client(struct rl_client* client);

bool
rl_update_client(struct rl_client* client,
                 struct inpt_state const* input,
                 float dt);

void
rl_render_client(struct rl_client const* client, SDL_Renderer* renderer);

#endif // GINC_ROGUELIKE_CLIENT_H
