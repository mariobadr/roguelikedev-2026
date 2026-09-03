#include "client.h"

#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

#include "controls.h"
#include "ui.h"
#include "view.h"

bool
rl_init_client(struct rl_client* client, SDL_Renderer* renderer)
{
  if (!rl_load_resources(&client->resources, renderer)) {
    rl_free_client(client);
    return false;
  }

  inpt_init_state(&client->istate);

  if (!rl_init_game_state(
        &client->game_state, RL_UI_MAP_WIDTH, RL_UI_MAP_HEIGHT)) {
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
  rl_destroy_resources(&client->resources);
}

void
rl_update_client(struct rl_client* client, float dt)
{
  struct rl_entity const* rogue =
    rl_get_entity(&client->game_state.world, RL_ROGUE_ID);
  enum rl_action const action =
    rl_translate_input(&client->istate, rogue->pos);
  rl_update_game_state(&client->game_state, action, dt);

  for (int i = 0; i < alist_len(&client->game_state.events); i++) {
    struct rl_event const* event = alist_at(&client->game_state.events, i);
    rl_game_log_on_event(&client->log, event, &client->game_state.world);
  }
}

void
rl_render_client(struct rl_client* client, SDL_Renderer* renderer)
{
  rl_render_game(renderer, client);
}
