#include "client/screen.h"

#include <SDL3/SDL_assert.h>

#include "game/game_state.h"

#include "ui/rectcut.h"

#include "client/action.h"
#include "client/controls.h"
#include "client/font.h"
#include "client/game_log.h"
#include "client/ui.h"
#include "client/ui_view.h"
#include "client/view/map.h"

/** The horizontal margin between UI panels, in logical pixels. */
#define RL_UI_MARGIN_X 4.0f
/** The vertical margin between UI panels, in logical pixels. */
#define RL_UI_MARGIN_Y 4.0f

struct rl_ui_layout
{
  SDL_FRect map_panel;
  SDL_FRect top_panel;
  SDL_FRect bottom_panel;
  SDL_FRect right_panel;
};

struct screen_state
{
  struct rl_font const* font;
  /** Time before the next action fires. */
  float action_cooldown;

  // Game state
  struct rl_game_state game_state;

  // Model state
  struct rl_game_log log;

  // View state
  struct rl_ui_layout layout;
  struct rl_map_view map_view;
};

/**
 * Roughly:
 *
 * ┌──────────────────────────────┬───────────────┐
 * │          top panel           │               │
 * ├──────────────────────────────┤               │
 * │                              │  right panel  │
 * │            main              │               │
 * │                              │               │
 * ├──────────────────────────────┤               │
 * │         bottom panel         │               │
 * └──────────────────────────────┴───────────────┘
 */
static struct rl_ui_layout
create_layout(void)
{
  struct rl_ui_layout layout = { 0 };
  SDL_FRect screen = { 0.0f, 0.0f, (float)RL_UI_WIDTH, (float)RL_UI_HEIGHT };

  ui_cut_left(&screen, RL_UI_MARGIN_X);
  SDL_FRect left = ui_cut_left(&screen, 384.0f);
  ui_cut_left(&screen, RL_UI_MARGIN_X);
  layout.right_panel = screen;

  // One line of text.
  layout.top_panel = ui_cut_top(&left, 8.0f);
  ui_cut_top(&left, RL_UI_MARGIN_Y);
  // Must stay a whole number of cells (see rl_draw_map).
  layout.map_panel = ui_cut_top(&left, 288.0f);
  ui_cut_top(&left, RL_UI_MARGIN_Y);
  layout.bottom_panel = left;

  return layout;
}

static bool
alloc_screen(struct screen_state* s, struct rl_font const* font)
{
  s->layout = create_layout();

  rl_init_map_view(
    &s->map_view, &s->layout.map_panel, font->glyph_width, font->glyph_height);

  int width, height;
  rl_map_view_size(&s->map_view, &width, &height);

  // TODO: add a camera at some point?
  if (!rl_alloc_game_state(&s->game_state, width, height)) {
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
    rl_translate_input(istate, rogue->pos, &s->map_view);
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
    rl_log_event(&s->log, event, &s->game_state.world);
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

  rl_draw_map_view(
    &s->map_view, renderer, s->font, &s->game_state.world, &s->game_state.fov);
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
