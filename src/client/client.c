#include "client.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>

#include "palette.h"

static struct rl_screen const*
top_screen(struct rl_client const* client)
{
  if (array_empty(&client->stack)) {
    return NULL;
  }

  size_t const top = array_len(&client->stack) - 1;
  enum rl_screen_id const id = *array_at(&client->stack, top);

  return &client->screens[id];
}

static void
push_screen(struct rl_client* client, enum rl_screen_id id)
{
  SDL_assert(!array_full(&client->stack));

  // exit the current screen, if it exists
  struct rl_screen const* prev = top_screen(client);
  if (prev != NULL) {
    prev->exit(prev->state);
  }

  // push the new id onto the stack
  *array_push(&client->stack) = id;

  // enter the newly pushed screen
  struct rl_screen const* next = top_screen(client);
  SDL_assert(next != NULL);
  next->enter(next->state);
}

static void
pop_screen(struct rl_client* client)
{
  SDL_assert(!array_empty(&client->stack));

  // exit the current screen, which must exist
  struct rl_screen const* prev = top_screen(client);
  SDL_assert(prev != NULL);
  prev->exit(prev->state);

  // pop the current screen off the stack
  array_pop(&client->stack);

  // enter the next current screen, if it exists
  struct rl_screen const* next = top_screen(client);
  if (next != NULL) {
    next->enter(next->state);
  }
}

static void
swap_top_screen(struct rl_client* client, enum rl_screen_id id)
{
  SDL_assert(!array_empty(&client->stack));

  // exit the current screen, which must exist
  struct rl_screen const* prev = top_screen(client);
  SDL_assert(prev != NULL);

  prev->exit(prev->state);
  array_pop(&client->stack);

  // push the new id onto the stack
  *array_push(&client->stack) = id;

  // enter the newly pushed screen
  struct rl_screen const* next = top_screen(client);
  SDL_assert(next != NULL);
  next->enter(next->state);
}

/** Apply a transition after update, preserving allocated screen state. */
static void
apply_transition(struct rl_client* client,
                 struct rl_screen_transition transition)
{
  switch (transition.type) {
    case RL_SCREEN_TRANSITION_NONE:
      break; // no transition to apply
    case RL_SCREEN_TRANSITION_PUSH:
      push_screen(client, transition.target);
      break;
    case RL_SCREEN_TRANSITION_POP:
      pop_screen(client);
      break;
    case RL_SCREEN_TRANSITION_SWAP:
      swap_top_screen(client, transition.target);
      break;
    default:
      break;
  }
}

bool
rl_init_client(struct rl_client* client, SDL_Renderer* renderer)
{
  if (!rl_load_font(&client->font, renderer)) {
    return false;
  }

  if (!array_alloc(&client->stack, RL_SCREEN_COUNT)) {
    SDL_Log("array_alloc failed: %s", SDL_GetError());
    return false;
  }

  if (!rl_alloc_gameplay_screen(&client->screens[RL_SCREEN_GAMEPLAY],
                                &client->font)) {
    return false;
  }

  struct rl_screen_transition transition = { 0 };
  transition.type = RL_SCREEN_TRANSITION_PUSH;
  transition.target = RL_SCREEN_GAMEPLAY;
  apply_transition(client, transition);

  return true;
}

void
rl_free_client(struct rl_client* client)
{
  if (client == NULL) {
    return;
  }

  // if there's a screen that's active, let it exit
  struct rl_screen const* active = top_screen(client);
  if (active != NULL) {
    active->exit(active->state);
  }

  // free the screens and their stack
  array_free(&client->stack);
  for (int i = 0; i < RL_SCREEN_COUNT; i++) {
    struct rl_screen* screen = &client->screens[i];
    if (screen->free != NULL) {
      screen->free(screen->state);
    }
  }

  // free the resources
  rl_unload_font(&client->font);
}

bool
rl_update_client(struct rl_client* client,
                 struct inpt_state const* input,
                 float dt)
{
  struct rl_screen const* screen = top_screen(client);
  SDL_assert(screen != NULL);

  struct rl_screen_transition const transition =
    screen->update(screen->state, input, dt);
  apply_transition(client, transition);

  return !array_empty(&client->stack);
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

  struct rl_screen const* screen = top_screen(client);
  screen->render(screen->state, renderer);
}
