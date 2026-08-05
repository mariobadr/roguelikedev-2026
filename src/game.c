#include "game.h"

#include <SDL3/SDL_render.h>

bool
rl_init_game(struct rl_game* game)
{
  (void)game;

  return true;
}

bool
rl_handle_input(struct rl_game* game, struct inpt_state const* istate)
{
  (void)game;
  (void)istate;

  return true;
}

void
rl_update_game(struct rl_game* game, float dt)
{
  (void)game;
  (void)dt;
}

void
rl_render_game(struct rl_game* game, SDL_Renderer* renderer)
{
  (void)game;

  SDL_SetRenderDrawColor(renderer, 16, 16, 16, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);
}
