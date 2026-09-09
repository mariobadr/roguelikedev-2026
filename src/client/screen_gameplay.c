#include "screen.h"

#include <SDL3/SDL_assert.h>

#include "game/game_state.h"

#include "action.h"
#include "cell.h"
#include "controls.h"
#include "game_log.h"
#include "map_view.h"
#include "ui.h"
#include "ui_view.h"

struct screen_state
{
  struct rl_font const* font;

  /** Time before the next action fires. */
  float action_cooldown;

  struct rl_game_state game_state;
  struct rl_game_log log;
  struct rl_ui_layout layout;
};

static bool
alloc_screen(struct screen_state* s, struct rl_font const* font)
{
  rl_init_ui_layout(&s->layout);

  // TODO: this is temporary
  int const main_width = (int)(s->layout.main_panel.w / rl_cell_width());
  int const main_height = (int)(s->layout.main_panel.h / rl_cell_height());
  if (!rl_alloc_game_state(&s->game_state, main_width, main_height)) {
    return false;
  }

  if (!rl_init_game_log(&s->log)) {
    return false;
  }

  s->action_cooldown = 0.0f;
  s->font = font;

  return true;
}

static void
free_screen(void* data)
{
  struct screen_state* s = (struct screen_state*)data;
  if (s == NULL) {
    return;
  }

  rl_free_game_log(&s->log);
  rl_free_game_state(&s->game_state);
  SDL_free(s);
}

static void
enter_screen(void* data)
{
  (void)data;
}

static void
exit_screen(void* data)
{
  (void)data;
}

static struct rl_screen_transition
update_screen(void* data, struct inpt_state const* istate, float dt)
{
  struct rl_screen_transition transition = { 0 };
  struct screen_state* s = (struct screen_state*)data;
  SDL_assert(s != NULL);

  s->action_cooldown = SDL_max(0.0f, s->action_cooldown - dt);
  if (s->action_cooldown > 0.0f) {
    return transition;
  }

  struct rl_actor const* rogue =
    rl_get_actor(&s->game_state.world, RL_ROGUE_ID);
  enum rl_action const action =
    rl_translate_input(istate, rogue->pos, &s->layout.main_panel);
  if (action == RL_ACTION_NONE) {
    return transition;
  }

  struct rl_command cmd =
    rl_build_command(RL_ROGUE_ID, action, &s->game_state.world);
  if (rl_update_game_state(&s->game_state, &cmd)) {
    s->action_cooldown = ACTION_GLOBAL_COOLDOWN;
  }

  // Consume this update's events exactly once, after submitting a command.
  for (int i = 0; i < alist_len(&s->game_state.events); i++) {
    struct rl_event const* event = alist_at(&s->game_state.events, i);
    rl_game_log_on_event(&s->log, event, &s->game_state.world);
  }

  return transition;
}

static void
render_screen(void const* data, SDL_Renderer* renderer)
{
  struct screen_state const* s = (struct screen_state*)data;
  SDL_assert(s != NULL);

  struct rl_actor const* rogue =
    rl_get_actor(&s->game_state.world, RL_ROGUE_ID);

  rl_draw_map(renderer,
              s->font,
              &s->layout.main_panel,
              &s->game_state.world,
              &s->game_state.fov);
  rl_draw_log(renderer, s->font, &s->layout.bottom_panel, &s->log);
  rl_draw_status(renderer, s->font, &s->layout.right_panel, rogue);
  rl_draw_controls(renderer, s->font, &s->layout.top_panel);
}

bool
rl_alloc_gameplay_screen(struct rl_screen* screen, struct rl_font const* font)
{
  screen->state = SDL_calloc(1, sizeof(struct screen_state));
  if (screen->state == NULL) {
    return false;
  }

  if (!alloc_screen(screen->state, font)) {
    free_screen(screen->state);
    screen->state = NULL;
    return false;
  }

  screen->free = free_screen;
  screen->enter = enter_screen;
  screen->exit = exit_screen;
  screen->update = update_screen;
  screen->render = render_screen;

  return true;
}
