#include "client.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>

#include "action.h"
#include "controls.h"
#include "palette.h"
#include "ui.h"
#include "view.h"

bool
rl_init_client(struct rl_client* client, SDL_Renderer* renderer)
{
  if (!rl_load_font(&client->font, renderer)) {
    rl_free_client(client);
    return false;
  }

  client->action_cooldown = 0.0f;

  rl_init_ui_layout(&client->layout);

  if (!rl_alloc_game_state(&client->game_state,
                           client->layout.main_panel.w,
                           client->layout.main_panel.h)) {
    rl_free_client(client);
    return false;
  }

  if (!rl_init_game_log(&client->log)) {
    rl_free_client(client);
    return false;
  }

  return true;
}

void
rl_free_client(struct rl_client* client)
{
  if (client == NULL) {
    return;
  }

  rl_free_game_log(&client->log);
  rl_free_game_state(&client->game_state);
  rl_unload_font(&client->font);
}

void
rl_update_client(struct rl_client* client,
                  struct inpt_state const* input,
                  float dt)
{
  client->action_cooldown = SDL_max(0.0f, client->action_cooldown - dt);
  if (client->action_cooldown > 0.0f) {
    return;
  }

  struct rl_actor const* rogue =
    rl_get_actor(&client->game_state.world, RL_ROGUE_ID);
  enum rl_action const action =
    rl_translate_input(input, rogue->pos, &client->layout.main_panel);
  if (action == RL_ACTION_NONE) {
    return;
  }

  struct rl_command cmd =
    rl_build_command(RL_ROGUE_ID, action, &client->game_state.world);
  if (rl_update_game_state(&client->game_state, &cmd)) {
    client->action_cooldown = ACTION_GLOBAL_COOLDOWN;
  }

  // Consume this update's events exactly once, after submitting a command.
  for (int i = 0; i < alist_len(&client->game_state.events); i++) {
    struct rl_event const* event = alist_at(&client->game_state.events, i);
    rl_game_log_on_event(&client->log, event, &client->game_state.world);
  }
}

void
rl_render_client(struct rl_client const* client, SDL_Renderer* renderer)
{
  SDL_SetRenderDrawColorFloat(renderer,
                              RL_COLOUR_GRAY[9].r,
                              RL_COLOUR_GRAY[9].g,
                              RL_COLOUR_GRAY[9].b,
                              RL_COLOUR_GRAY[9].a);
  SDL_RenderClear(renderer);

  struct rl_actor const* rogue =
    rl_get_actor(&client->game_state.world, RL_ROGUE_ID);

  rl_draw_map(renderer,
              &client->font,
              &client->layout.main_panel,
              &client->game_state.world,
              &client->game_state.fov);
  rl_draw_log(
    renderer, &client->font, &client->layout.bottom_panel, &client->log);
  rl_draw_status(renderer, &client->font, &client->layout.right_panel, rogue);
  rl_draw_controls(renderer, &client->font, &client->layout.top_panel);
}
