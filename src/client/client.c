#include "client.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>

#include "ui/rectcut.h"

#include "render/palette.h"

#include "client/screen/game_over.h"
#include "client/screen/gameplay.h"
#include "client/screen/main_menu.h"
#include "client/screen/save_files.h"

/** The gap between the ribbon and the screen content, in logical pixels. */
#define RIBBON_GAP_Y 4.0f
/** The ribbon's inset from the left edge of the bounds, in logical pixels. */
#define RIBBON_INSET_X 4.0f

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
  }
}

// Cuts the ribbon's strip off the top of the bounds, leaving the content.
static SDL_FRect
cut_ribbon_viewport(SDL_FRect* bounds, struct gfx_tileset const* font)
{
  SDL_FRect viewport = ui_cut_top(bounds, (float)font->tile_height);
  ui_cut_top(bounds, RIBBON_GAP_Y);
  ui_cut_left(&viewport, RIBBON_INSET_X);

  return viewport;
}

static void
refresh_ribbon(struct rl_client* client)
{
  struct rl_ribbon_content content = { 0 };

  struct rl_screen const* screen = top_screen(client);
  if (screen != NULL) {
    rl_screen_describe_ribbon(screen, &content);
  }

  rl_set_ribbon_content(&client->ribbon, &content);
}

bool
rl_alloc_client(struct rl_client* client,
                SDL_FRect const* bounds,
                SDL_Renderer* renderer)
{
  if (!rl_load_font(&client->font, renderer)) {
    return false;
  }

  SDL_FRect content_bounds = *bounds;
  SDL_FRect const ribbon_viewport =
    cut_ribbon_viewport(&content_bounds, &client->font);
  rl_init_ribbon(&client->ribbon, &ribbon_viewport, &client->font);

  if (!array_alloc(&client->stack, RL_SCREEN_COUNT)) {
    SDL_Log("array_alloc failed: %s", SDL_GetError());
    return false;
  }

  if (!rl_alloc_main_menu_screen(&client->screens[RL_SCREEN_MAIN_MENU],
                                 &content_bounds,
                                 &client->font,
                                 &client->run)) {
    return false;
  }

  if (!rl_alloc_save_files_screen(&client->screens[RL_SCREEN_SAVE_FILES],
                                  &content_bounds,
                                  &client->font,
                                  &client->run)) {
    return false;
  }

  if (!rl_alloc_gameplay_screen(&client->screens[RL_SCREEN_GAMEPLAY],
                                &content_bounds,
                                &client->font,
                                &client->run)) {
    return false;
  }

  if (!rl_alloc_game_over_screen(&client->screens[RL_SCREEN_GAME_OVER],
                                 &content_bounds,
                                 &client->font)) {
    return false;
  }

  struct rl_screen_transition transition = { 0 };
  transition.type = RL_SCREEN_TRANSITION_PUSH;
  transition.target = RL_SCREEN_MAIN_MENU;
  apply_transition(client, transition);
  refresh_ribbon(client);

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

  // free the resources borrowed by the screens
  rl_free_run(&client->run);
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
  refresh_ribbon(client);

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

  rl_draw_ribbon(&client->ribbon, renderer, &client->font);
}

void
rl_exit_client(struct rl_client* client)
{
  if (rl_save_id_is_valid(client->run.save_id)) {
    enum rl_save_result result = rl_save_run(&client->run);
    if (result == RL_SAVE_OK) {
      SDL_Log("Game was saved on exit.");
    }
  }
}
