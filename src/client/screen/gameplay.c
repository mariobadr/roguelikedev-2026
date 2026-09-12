#include "client/screen.h"

#include <SDL3/SDL_assert.h>

#include "game/game_state.h"

#include "ui/rectcut.h"

#include "client/view/in_sight.h"
#include "client/view/inventory.h"
#include "client/view/log.h"
#include "client/view/map.h"
#include "client/view/ribbon.h"

#include "client/action.h"
#include "client/controls.h"
#include "client/font.h"
#include "client/game_log.h"
#include "client/ui.h"
#include "client/view.h"

/** The horizontal margin between UI panels, in logical pixels. */
#define RL_UI_MARGIN_X 4.0f
/** The vertical margin between UI panels, in logical pixels. */
#define RL_UI_MARGIN_Y 4.0f
/** How long between subsequent keys. */
#define KEY_REPEAT_COOLDOWN (0.115f)

#define RL_WORLD_WIDTH 64
#define RL_WORLD_HEIGHT 64

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
  enum rl_view_id panel_views[PANEL_COUNT];
  enum panel_id focused_panel;

  // Game state
  struct rl_game_state game_state;
  struct rl_command pending_target_cmd; // RL_COMMAND_NONE when idle

  // Model state
  struct rl_game_log log;

  // View state
  struct rl_view views[RL_VIEW_COUNT];
  struct rl_ribbon ribbon;
};

/**
 * Roughly:
 *
 * ┌──────────────────────────────────────────────┐
 * │                  top panel                   │
 * ├──────────────────────────────┬───────────────┤
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

  // One line of text across the full available width.
  panels[PANEL_TOP] = ui_cut_top(&screen, 8.0f);
  ui_cut_top(&screen, RL_UI_MARGIN_Y);

  SDL_FRect left = ui_cut_left(&screen, 384.0f);
  ui_cut_left(&screen, RL_UI_MARGIN_X);
  panels[PANEL_RIGHT] = screen;

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

  // this is the initial mapping; PANEL_TOP always shows the ribbon, which
  // isn't a view, so it has no entry here
  s->panel_views[PANEL_MAIN] = RL_VIEW_MAP;
  s->panel_views[PANEL_BOTTOM] = RL_VIEW_LOG;
  s->panel_views[PANEL_RIGHT] = RL_VIEW_IN_SIGHT;

  if (!rl_alloc_in_sight_view(&s->views[RL_VIEW_IN_SIGHT],
                              &s->game_state.world,
                              &s->game_state.fov,
                              &s->panel_bounds[PANEL_RIGHT])) {
    return false;
  }

  if (!rl_alloc_log_view(&s->views[RL_VIEW_LOG],
                         &s->log,
                         &s->panel_bounds[PANEL_BOTTOM],
                         (float)font->glyph_height)) {
    return false;
  }

  if (!rl_alloc_inv_view(&s->views[RL_VIEW_INVENTORY],
                         &s->game_state.world,
                         &s->panel_bounds[PANEL_BOTTOM],
                         (float)font->glyph_height)) {
    return false;
  }

  // the map is only designed to work in the main panel right now
  if (!rl_alloc_map_view(&s->views[RL_VIEW_MAP],
                         &s->game_state.world,
                         &s->game_state.fov,
                         &s->panel_bounds[PANEL_MAIN],
                         font->glyph_width,
                         font->glyph_height)) {
    return false;
  }

  // ribbon
  rl_init_ribbon(&s->ribbon, &s->panel_bounds[PANEL_TOP]);

  if (!rl_alloc_game_state(&s->game_state, RL_WORLD_WIDTH, RL_WORLD_HEIGHT)) {
    return false;
  }

  if (!rl_init_game_log(&s->log)) {
    return false;
  }

  s->repeat_cooldown = 0.0f;
  s->font = font;

  s->focused_panel = PANEL_MAIN;
  rl_view_update_ribbon(&s->views[s->panel_views[s->focused_panel]],
                        &s->ribbon);

  return true;
}

static void
free_screen(void* data)
{
  struct screen_state* s = (struct screen_state*)data;
  if (s == NULL) {
    return;
  }

  for (int i = 0; i < RL_VIEW_COUNT; ++i) {
    rl_free_view(&s->views[i]);
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
cancel_focus(struct screen_state* s)
{
  s->focused_panel = PANEL_MAIN;
}

static void
show_inventory(struct screen_state* s)
{
  if (s->focused_panel == PANEL_BOTTOM &&
      s->panel_views[PANEL_BOTTOM] == RL_VIEW_INVENTORY) {
    cancel_focus(s);
    return;
  }

  s->panel_views[PANEL_BOTTOM] = RL_VIEW_INVENTORY;
  s->focused_panel = PANEL_BOTTOM;
}

static void
show_log(struct screen_state* s)
{
  if (s->focused_panel == PANEL_BOTTOM &&
      s->panel_views[PANEL_BOTTOM] == RL_VIEW_LOG) {
    cancel_focus(s);
    return;
  }

  s->panel_views[PANEL_BOTTOM] = RL_VIEW_LOG;
  s->focused_panel = PANEL_BOTTOM;
}

static bool
submit_command(struct screen_state* s, struct rl_command const* cmd)
{
  bool const handled = rl_update_game_state(&s->game_state, cmd);

  // Consume this update's events exactly once, after submitting a command.
  for (int i = 0; i < alist_len(&s->game_state.events); i++) {
    struct rl_event const* event = alist_at(&s->game_state.events, i);
    rl_log_event(&s->log, event, &s->game_state.world);
  }

  return handled;
}

static void
begin_target_select(struct screen_state* s, int item_id)
{
  struct rl_actor const* rogue =
    rl_get_actor(&s->game_state.world, RL_ROGUE_ID);

  s->pending_target_cmd = (struct rl_command){ 0 };
  s->pending_target_cmd.actor = RL_ROGUE_ID;
  s->pending_target_cmd.type = RL_COMMAND_USE_ITEM;
  s->pending_target_cmd.use_item.item_id = item_id;

  rl_map_view_begin_select(&s->views[RL_VIEW_MAP], rogue->pos);
  s->focused_panel = PANEL_MAIN;
}

static void
resolve_pending_target(struct screen_state* s)
{
  if (s->pending_target_cmd.type == RL_COMMAND_NONE) {
    return;
  }

  SDL_Point dst;
  enum rl_map_selection_result result =
    rl_map_view_take_selection(&s->views[RL_VIEW_MAP], &dst);

  switch (result) {
    case RL_MAP_SELECTION_CONFIRMED:
      s->pending_target_cmd.use_item.dst = dst;
      submit_command(s, &s->pending_target_cmd);
      s->pending_target_cmd = (struct rl_command){ 0 };
      break;
    case RL_MAP_SELECTION_CANCELLED:
      s->pending_target_cmd = (struct rl_command){ 0 };
      break;
    case RL_MAP_SELECTION_NONE:
      break;
  }
}

static bool
handle_item_selection(struct screen_state* s, int item_id)
{
  if (item_id < 0) {
    return false;
  }

  struct rl_item const* item = rl_get_item(&s->game_state.world, item_id);
  if (item == NULL) {
    return false;
  }

  struct rl_item_def const* def = rl_get_item_def(item->itype);
  bool handled = false;

  switch (def->target) {
    case RL_ITEM_TARGET_NONE:
      // use item
      struct rl_command cmd = { 0 };
      cmd.actor = RL_ROGUE_ID;
      cmd.type = RL_COMMAND_USE_ITEM;
      cmd.use_item.item_id = item_id;
      handled = submit_command(s, &cmd);
      break;
    case RL_ITEM_TARGET_TILE:
      begin_target_select(s, item_id);
      handled = true;
      break;
  }

  return handled;
}

static bool
handle_action(struct screen_state* s, struct inpt_state const* istate)
{
  enum rl_view_id const view_id = s->panel_views[s->focused_panel];
  struct rl_view* view = &s->views[view_id];

  bool handled = rl_update_view(view, istate);

  switch (view_id) {
    case RL_VIEW_INVENTORY:
      int item_id = rl_inv_view_take_selection(view);
      handled = handle_item_selection(s, item_id) || handled;
      break;
    case RL_VIEW_MAP: {
      struct rl_command cmd;
      if (rl_map_view_take_command(view, &cmd)) {
        handled = submit_command(s, &cmd) || handled;
      }
      break;
    }
    default:
      break;
  }

  if (handled) {
    s->repeat_cooldown = KEY_REPEAT_COOLDOWN;
  }

  return handled;
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

  if (s->pending_target_cmd.type != RL_COMMAND_NONE) {
    // Targeting a tile, don't interrupt
    handle_action(s, istate);
  } else if (action == RL_ACTION_SHOW_INVENTORY) {
    show_inventory(s);
  } else if (action == RL_ACTION_SHOW_LOG) {
    show_log(s);
  } else if (action == RL_ACTION_CANCEL) {
    if (!handle_action(s, istate)) {
      cancel_focus(s);
    }
  } else {
    handle_action(s, istate);
  }

  resolve_pending_target(s);

  for (int i = 0; i < RL_VIEW_COUNT; ++i) {
    rl_prepare_view(&s->views[i]);
  }

  rl_view_update_ribbon(&s->views[s->panel_views[s->focused_panel]],
                        &s->ribbon);
  return transition;
}

static void
render_screen(void const* data, SDL_Renderer* renderer)
{
  struct screen_state const* s = (struct screen_state*)data;
  SDL_assert(s != NULL);

  for (int panel = 0; panel < PANEL_COUNT; ++panel) {
    if (panel == PANEL_TOP) {
      rl_draw_ribbon(&s->ribbon, renderer, s->font);
      continue;
    }

    rl_render_view(&s->views[s->panel_views[panel]], renderer, s->font);
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
