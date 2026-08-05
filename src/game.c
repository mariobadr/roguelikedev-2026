#include "game.h"

#include <SDL3/SDL_render.h>

#include "input.h"

static void
set_movement_action(struct rl_action* action, int x, int y)
{
  action->type = RL_ACTION_MOVE;
  action->move_vector.x = x;
  action->move_vector.y = y;
}

bool
rl_init_game(struct rl_game* game)
{
  game->player.x = 0;
  game->player.y = 0;
  game->action.type = RL_ACTION_NONE;
  game->action_cooldown = 0.0f;

  return true;
}

bool
rl_handle_input(struct rl_game* game, struct inpt_state const* istate)
{
  // reset the action for this frame
  game->action.type = RL_ACTION_NONE;

  if (game->action_cooldown > 0.0f) {
    // still recovering from the last action
    return true;
  }

  if (inpt_is_down(istate->keys[SDL_SCANCODE_W])) {
    set_movement_action(&game->action, 0, -1);
  } else if (inpt_is_down(istate->keys[SDL_SCANCODE_S])) {
    set_movement_action(&game->action, 0, 1);
  } else if (inpt_is_down(istate->keys[SDL_SCANCODE_A])) {
    set_movement_action(&game->action, -1, 0);
  } else if (inpt_is_down(istate->keys[SDL_SCANCODE_D])) {
    set_movement_action(&game->action, 1, 0);
  }

  return true;
}

void
rl_update_game(struct rl_game* game, float dt)
{
  if (game->action_cooldown > 0.0f) {
    game->action_cooldown -= dt;
  }

  if (game->action.type == RL_ACTION_MOVE) {
    game->player.x += game->action.move_vector.x;
    game->player.y += game->action.move_vector.y;
    game->action_cooldown = 0.115f;
  }
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
