#include "game.h"

#include <SDL3/SDL_render.h>

bool
rl_init_game(struct rl_game* game)
{
  game->player.x = 0;
  game->player.y = 0;

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
  SDL_SetRenderDrawColor(renderer, 16, 16, 16, SDL_ALPHA_OPAQUE);
  SDL_RenderClear(renderer);

  // convert from tile to screen coordinates
  float const player_x = game->player.x * 8.0f;
  float const player_y = game->player.y * 8.0f;

  // draw the rogue
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, SDL_ALPHA_OPAQUE);
  SDL_RenderDebugText(renderer, player_x, player_y, "@");
}
