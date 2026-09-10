#include "client/screen.h"

#include <SDL3/SDL_assert.h>

#include "game/game_state.h"

#include "ui/rectcut.h"

#include "client/view/inventory.h"
#include "client/view/log.h"
#include "client/view/map.h"
#include "client/view/ribbon.h"

#include "client/action.h"
#include "client/controls.h"
#include "client/font.h"
#include "client/game_log.h"
#include "client/ui.h"
#include "client/ui_view.h"

/** The horizontal margin between UI panels, in logical pixels. */
#define RL_UI_MARGIN_X 4.0f
/** The vertical margin between UI panels, in logical pixels. */
#define RL_UI_MARGIN_Y 4.0f
/** How long between subsequent keys. */
#define KEY_REPEAT_COOLDOWN (0.115f)

// I'm not sure if this stays in screen
enum view_id
{
  VIEW_MAP,
  VIEW_LOG,
  VIEW_INVENTORY,
  VIEW_CONTROLS,
  VIEW_STATUS,
  VIEW_COUNT,
};

// I think this stays in screen
enum panel_id
{
  PANEL_MAIN,
  PANEL_TOP,
  PANEL_BOTTOM,
  PANEL_RIGHT,
  PANEL_COUNT
};

struct screen_state
{
  struct rl_font const* font;
  float repeat_cooldown;

  // UI state
  SDL_FRect panel_bounds[PANEL_COUNT];
  enum view_id panel_views[PANEL_COUNT];
  enum panel_id focused_panel;

  // Game state
  struct rl_game_state game_state;

  // Model state
  struct rl_game_log log;

  // View state
  struct rl_inv_view inv_view;
  struct rl_log_view log_view;
  struct rl_map_view map_view;
  struct rl_ribbon ribbon;
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
static void
create_layout(SDL_FRect panels[PANEL_COUNT])
{
  SDL_FRect screen = { 0.0f, 0.0f, (float)RL_UI_WIDTH, (float)RL_UI_HEIGHT };

  ui_cut_left(&screen, RL_UI_MARGIN_X);
  SDL_FRect left = ui_cut_left(&screen, 384.0f);
  ui_cut_left(&screen, RL_UI_MARGIN_X);
  panels[PANEL_RIGHT] = screen;

  // One line of text.
  panels[PANEL_TOP] = ui_cut_top(&left, 8.0f);
  ui_cut_top(&left, RL_UI_MARGIN_Y);
  // Must stay a whole number of cells (see rl_draw_map).
  panels[PANEL_MAIN] = ui_cut_top(&left, 288.0f);
  ui_cut_top(&left, RL_UI_MARGIN_Y);
  panels[PANEL_BOTTOM] = left;
}

static bool
alloc_screen(struct screen_state* s, struct rl_font const* font)
{
  // the layout is fixed
  create_layout(s->panel_bounds);

  // this is the initial mapping
  s->panel_views[PANEL_MAIN] = VIEW_MAP;
  s->panel_views[PANEL_TOP] = VIEW_CONTROLS;
  s->panel_views[PANEL_BOTTOM] = VIEW_LOG;
  s->panel_views[PANEL_RIGHT] = VIEW_STATUS;

  // bottom panel views
  rl_init_log_view(
    &s->log_view, &s->panel_bounds[PANEL_BOTTOM], (float)font->glyph_height);
  rl_init_inv_view(
    &s->inv_view, &s->panel_bounds[PANEL_BOTTOM], (float)font->glyph_height);

  // the map is only designed to work in the main panel right now
  rl_init_map_view(&s->map_view,
                   &s->panel_bounds[PANEL_MAIN],
                   font->glyph_width,
                   font->glyph_height);

  // ribbon
  rl_init_ribbon(&s->ribbon, &s->panel_bounds[PANEL_TOP]);

  int width, height;
  rl_map_view_size(&s->map_view, &width, &height);

  // TODO: add a camera at some point?
  if (!rl_alloc_game_state(&s->game_state, width, height)) {
    return false;
  }

  if (!rl_init_game_log(&s->log)) {
    return false;
  }

  s->repeat_cooldown = 0.0f;
  s->font = font;

  s->focused_panel = PANEL_MAIN;
  rl_map_view_ribbon(&s->map_view, &s->ribbon);

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

static void
update_ribbon(struct screen_state* s)
{
  switch (s->panel_views[s->focused_panel]) {
    case VIEW_MAP:
      rl_map_view_ribbon(&s->map_view, &s->ribbon);
      break;
    case VIEW_LOG:
      rl_log_view_ribbon(&s->log_view, &s->ribbon);
      break;
    case VIEW_INVENTORY:
      rl_inv_view_ribbon(&s->inv_view, &s->ribbon);
      break;
    default:
      break;
  }
}

static bool
view_is_focusable(enum view_id view)
{
  switch (view) {
    case VIEW_INVENTORY:
    case VIEW_LOG:
    case VIEW_MAP:
      return true;
    case VIEW_CONTROLS:
    case VIEW_STATUS:
      return false;
    default:
      break;
  }

  return false;
}

static enum panel_id
next_focusable_panel(struct screen_state const* s)
{
  enum panel_id next = s->focused_panel;
  do {
    next = (enum panel_id)((next + 1) % PANEL_COUNT);
  } while (!view_is_focusable(s->panel_views[next]) &&
           next != s->focused_panel);
  return next;
}

static void
toggle_inventory(struct screen_state* s)
{
  if (s->panel_views[PANEL_BOTTOM] == VIEW_INVENTORY) {
    s->panel_views[PANEL_BOTTOM] = VIEW_LOG;
    s->focused_panel = PANEL_MAIN;
    return;
  }

  if (s->panel_views[PANEL_BOTTOM] == VIEW_LOG) {
    s->panel_views[PANEL_BOTTOM] = VIEW_INVENTORY;
    s->focused_panel = PANEL_BOTTOM;
    return;
  }
}

static bool
handle_inv_action(struct screen_state* s, enum rl_action action)
{
  switch (action) {
    case RL_ACTION_MOVE_UP:
      rl_select_inv_view_up(&s->inv_view, &s->game_state.world);
      return true;
    case RL_ACTION_MOVE_DOWN:
      rl_select_inv_view_down(&s->inv_view, &s->game_state.world);
      return true;
    default:
      break;
  }

  return false;
}

static bool
handle_log_action(struct screen_state* s, enum rl_action action)
{
  switch (action) {
    case RL_ACTION_MOVE_UP:
      rl_scroll_log_view_up(&s->log_view, &s->log);
      return true;
    case RL_ACTION_MOVE_DOWN:
      rl_scroll_log_view_down(&s->log_view, &s->log);
      return true;
    default:
      break;
  }

  return false;
}

static bool
handle_map_action(struct screen_state* s, enum rl_action action)
{
  if (action == RL_ACTION_NONE) {
    return false;
  }
  struct rl_command cmd =
    rl_build_command(RL_ROGUE_ID, action, &s->game_state.world);
  bool const handled = rl_update_game_state(&s->game_state, &cmd);

  // Consume this update's events exactly once, after submitting a command.
  for (int i = 0; i < alist_len(&s->game_state.events); i++) {
    struct rl_event const* event = alist_at(&s->game_state.events, i);
    rl_log_event(&s->log, event, &s->game_state.world);
  }

  return handled;
}

static void
handle_action(struct screen_state* s, enum rl_action action)
{
  bool handled = false;

  switch (s->panel_views[s->focused_panel]) {
    case VIEW_INVENTORY:
      handled = handle_inv_action(s, action);
      break;
    case VIEW_LOG:
      handled = handle_log_action(s, action);
      break;
    case VIEW_MAP:
      handled = handle_map_action(s, action);
      break;
    case VIEW_STATUS:
      break;
    case VIEW_CONTROLS:
      break;
    default:
      break;
  }

  if (handled) {
    s->repeat_cooldown = KEY_REPEAT_COOLDOWN;
  }
}

static struct rl_screen_transition
update_screen(void* data, struct inpt_state const* istate, float dt)
{
  struct rl_screen_transition transition = { 0 };
  struct screen_state* s = (struct screen_state*)data;
  SDL_assert(s != NULL);

  s->repeat_cooldown = SDL_max(0.0f, s->repeat_cooldown - dt);
  if (s->repeat_cooldown > 0.0f) {
    return transition;
  }

  enum rl_action action = rl_handle_keyboard_input(istate);
  if (action == RL_ACTION_FOCUS_NEXT) {
    s->focused_panel = next_focusable_panel(s);
  } else if (action == RL_ACTION_TOGGLE_INVENTORY) {
    toggle_inventory(s);
  } else {
    enum view_id const view = s->panel_views[s->focused_panel];

    if (action == RL_ACTION_NONE && view == VIEW_MAP) {
      struct rl_actor const* rogue =
        rl_get_actor(&s->game_state.world, RL_ROGUE_ID);
      action = rl_handle_mouse_input(istate, rogue->pos, &s->map_view);
    }

    handle_action(s, action);
  }

  update_ribbon(s);
  return transition;
}

static void
render_screen(void const* data, SDL_Renderer* renderer)
{
  struct screen_state const* s = (struct screen_state*)data;
  SDL_assert(s != NULL);

  struct rl_actor const* rogue =
    rl_get_actor(&s->game_state.world, RL_ROGUE_ID);

  for (int panel = 0; panel < PANEL_COUNT; ++panel) {
    switch (s->panel_views[panel]) {
      case VIEW_MAP:
        rl_draw_map_view(&s->map_view,
                         renderer,
                         s->font,
                         &s->game_state.world,
                         &s->game_state.fov);
        break;
      case VIEW_LOG:
        rl_draw_log_view(&s->log_view, renderer, s->font, &s->log);
        break;
      case VIEW_INVENTORY:
        rl_draw_inv_view(&s->inv_view, renderer, s->font, &s->game_state.world);
        break;
      case VIEW_STATUS:
        rl_draw_status(renderer, s->font, &s->panel_bounds[panel], rogue);
        break;
      case VIEW_CONTROLS:
        rl_draw_ribbon(&s->ribbon, renderer, s->font);
        break;
      default:
        break;
    }
  }
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
