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

  if (!alist_alloc(&client->messages, 8)) {
    SDL_Log("alist_alloc failed: %s", SDL_GetError());
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

  alist_free(&client->messages);
  rl_free_game_state(&client->game_state);
  rl_destroy_resources(&client->resources);
}

void
rl_update_client(struct rl_client* client, float dt)
{
  struct rl_entity const* rogue =
    rl_get_entity(&client->game_state.world, RL_ROGUE_ID);
  enum rl_action const action =
    rl_translate_input(&client->istate, rogue->position);
  rl_update_game_state(&client->game_state, action, dt);

  for (int i = 0; i < alist_len(&client->game_state.events); i++) {
    struct rl_event const* event = alist_at(&client->game_state.events, i);

    switch (event->type) {
      case RL_EVENT_ATTACK:
        *alist_push(&client->messages) =
          rl_build_attack_log(&client->game_state.world, &event->as.attack);
        break;
      case RL_EVENT_DEATH:
        *alist_push(&client->messages) =
          rl_build_death_log(&client->game_state.world, &event->as.death);
        break;
      default:
        break;
    }
  }
}

void
rl_render_client(struct rl_client* client, SDL_Renderer* renderer)
{
  rl_render_game(renderer, client);
}
