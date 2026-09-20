/**
 * @file client.h
 */
#ifndef GINC_ROGUELIKE_CLIENT_H
#define GINC_ROGUELIKE_CLIENT_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "container/array.h"

#include "client/font.h"
#include "client/ribbon.h"
#include "client/run.h"
#include "client/screen.h"

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct inpt_state;

/**
 * A screen stack.
 */
array_define_as(enum rl_screen_id, rl_screen_id);

/**
 * The client application: its screens and the run they display.
 */
struct rl_client
{
  /** Bitmap font used to draw everything. */
  struct gfx_tileset font;
  /** Status line drawn above the active screen. */
  struct rl_ribbon ribbon;
  /** The active run. */
  struct rl_run run;
  /** Allocated screens, retained when removed from the stack. */
  struct rl_screen screens[RL_SCREEN_COUNT];
  /** Each screen may appear at most once; only the top is active. */
  array(rl_screen_id) stack;
};

/**
 * Allocate the client, drawing within bounds.
 *
 * @return whether allocation succeeded.
 */
bool
rl_alloc_client(struct rl_client* client,
                SDL_FRect const* bounds,
                SDL_Renderer* renderer);

/**
 * Free the client.
 */
void
rl_free_client(struct rl_client* client);

/**
 * Update the client for one frame.
 *
 * @return whether the client should keep running.
 */
bool
rl_update_client(struct rl_client* client,
                 struct inpt_state const* input,
                 float dt);

/**
 * Render the client.
 */
void
rl_render_client(struct rl_client const* client, SDL_Renderer* renderer);

/**
 * Save the run, if there is one, before the application exits.
 */
void
rl_exit_client(struct rl_client* client);

#endif // GINC_ROGUELIKE_CLIENT_H
