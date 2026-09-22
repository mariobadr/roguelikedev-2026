#include "gameplay.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_render.h>

#include "cp437/border.h"

#include "game/actor.h"
#include "game/game.h"
#include "game/item_def.h"

#include "graphics/console.h"

#include "ui/rectcut.h"

#include "render/border.h"
#include "render/palette.h"

#include "client/view/character.h"
#include "client/view/in_sight.h"
#include "client/view/inventory.h"
#include "client/view/log.h"
#include "client/view/world.h"

#include "client/action.h"
#include "client/controls.h"
#include "client/font.h"
#include "client/game_log.h"
#include "client/ribbon.h"
#include "client/run.h"
#include "client/screen.h"
#include "client/text.h"
#include "client/view.h"

/** The minimum bottom panel content height, in logical pixels. */
#define RL_BOTTOM_HEIGHT 56.0f
/** The minimum sidebar content width, in logical pixels. */
#define RL_SIDEBAR_WIDTH 88.0f
/** How long between subsequent keys. */
#define KEY_REPEAT_COOLDOWN (0.115f)

// I think this stays in screen
enum panel_id
{
  PANEL_MAIN,
  PANEL_BOTTOM,
  PANEL_RIGHT,
  PANEL_COUNT
};

/**
 * A labelled tab for a view in the bottom panel.
 */
struct bottom_tab
{
  /** The view displayed when the tab is selected. */
  enum rl_view_id view;
  /** The label and keyboard shortcut shown in the tab. */
  char const* label;
};

static struct bottom_tab const BOTTOM_TABS[] = {
  { .view = RL_VIEW_LOG, .label = "Log [L]" },
  { .view = RL_VIEW_INVENTORY, .label = "Inventory [I]" },
  { .view = RL_VIEW_CHARACTER, .label = "Character [C]" },
};

struct screen_state
{
  struct gfx_tileset const* font;
  float repeat_cooldown;

  // UI state
  /** The retained border grid shared by the gameplay panels. */
  grid(cp437_border) border_grid;
  SDL_FRect frame_bounds;
  SDL_FRect panel_bounds[PANEL_COUNT];
  /** The tab interiors, including horizontal padding, in logical pixels. */
  SDL_FRect tab_bounds[SDL_arraysize(BOTTOM_TABS)];
  enum rl_view_id panel_views[PANEL_COUNT];
  enum panel_id focused_panel;

  // Game state
  struct rl_run* run;
  struct rl_command pending_target_cmd;
  bool game_over;

  // Model state
  struct rl_game_log log;
  alist(rl_event) events;

  // View state
  struct rl_view views[RL_VIEW_COUNT];
};

/**
 * Roughly:
 *
 * ┌──────────────────────────────────────────────┬───────────────┐
 * │                                              │  right panel  │
 * │                    main                      │               │
 * │                                              │               │
 * ├─────────┬───────────────┬───────────────┬────┤               │
 * │ Log [L] │ Inventory [I] │ Character [C] │    │               │
 * ├─────────┴───────────────┴───────────────┴────┤               │
 * │                 bottom panel                 │               │
 * └──────────────────────────────────────────────┴───────────────┘
 */
static void
create_layout(struct screen_state* s,
              SDL_FRect const* bounds,
              struct gfx_tileset const* font)
{
  float const cell_w = (float)font->tile_width;
  float const cell_h = (float)font->tile_height;
  s->frame_bounds = *bounds;
  s->frame_bounds.w = SDL_floorf(bounds->w / cell_w) * cell_w;
  s->frame_bounds.h = SDL_floorf(bounds->h / cell_h) * cell_h;

  SDL_FRect content = s->frame_bounds;
  ui_cut_left(&content, cell_w);
  ui_cut_right(&content, cell_w);
  ui_cut_top(&content, cell_h);
  ui_cut_bottom(&content, cell_h);

  float const sidebar_w = SDL_ceilf(RL_SIDEBAR_WIDTH / cell_w) * cell_w;
  float const bottom_h = SDL_ceilf(RL_BOTTOM_HEIGHT / cell_h) * cell_h;
  s->panel_bounds[PANEL_RIGHT] = ui_cut_right(&content, sidebar_w);
  ui_cut_right(&content, cell_w);
  s->panel_bounds[PANEL_BOTTOM] = ui_cut_bottom(&content, bottom_h);
  ui_cut_bottom(&content, cell_h);
  SDL_FRect tabs = ui_cut_bottom(&content, cell_h);
  ui_cut_bottom(&content, cell_h);
  s->panel_bounds[PANEL_MAIN] = content;

  for (size_t tab = 0; tab < SDL_arraysize(BOTTOM_TABS); ++tab) {
    float const width =
      rl_font_width(font, SDL_strlen(BOTTOM_TABS[tab].label)) + 2.0f * cell_w;
    s->tab_bounds[tab] = ui_cut_left(&tabs, width);
    ui_cut_left(&tabs, cell_w);
  }
}

static void
add_border_box(struct screen_state* s,
               struct gfx_tileset const* font,
               SDL_FRect const* content,
               struct cp437_border_style const* style)
{
  SDL_Rect const rect = {
    (int)((content->x - s->frame_bounds.x) / font->tile_width) - 1,
    (int)((content->y - s->frame_bounds.y) / font->tile_height) - 1,
    (int)(content->w / font->tile_width) + 2,
    (int)(content->h / font->tile_height) + 2,
  };
  cp437_add_box(&s->border_grid, &rect, style);
}

static void
add_border_boxes(struct screen_state* s, struct gfx_tileset const* font)
{
  struct cp437_border_style b = { 0 };
  b.top = CP437_BORDER_SINGLE;
  b.bottom = CP437_BORDER_SINGLE;
  b.right = CP437_BORDER_SINGLE;
  b.left = CP437_BORDER_DOUBLE;

  add_border_box(s, font, &s->panel_bounds[PANEL_MAIN], &b);
  add_border_box(s, font, &s->panel_bounds[PANEL_BOTTOM], &b);
  add_border_box(s, font, &s->tab_bounds[0], &b);

  b.left = CP437_BORDER_SINGLE;
  b.right = CP437_BORDER_DOUBLE;
  add_border_box(s, font, &s->panel_bounds[PANEL_RIGHT], &b);

  b.left = CP437_BORDER_SINGLE;
  b.right = CP437_BORDER_SINGLE;
  for (size_t tab = 1; tab < SDL_arraysize(BOTTOM_TABS); ++tab) {
    add_border_box(s, font, &s->tab_bounds[tab], &b);
  }
}

static bool
alloc_screen(struct screen_state* s,
             SDL_FRect const* bounds,
             struct gfx_tileset const* font,
             struct rl_run* run)
{
  s->run = run;

  // the layout is fixed
  create_layout(s, bounds, font);

  if (!grid_alloc(&s->border_grid,
                  (int)(s->frame_bounds.w / font->tile_width),
                  (int)(s->frame_bounds.h / font->tile_height))) {
    return false;
  }

  add_border_boxes(s, font);

  // this is the initial mapping
  s->panel_views[PANEL_MAIN] = RL_VIEW_WORLD;
  s->panel_views[PANEL_BOTTOM] = RL_VIEW_LOG;
  s->panel_views[PANEL_RIGHT] = RL_VIEW_IN_SIGHT;

  if (!rl_alloc_in_sight_view(&s->views[RL_VIEW_IN_SIGHT],
                              &s->run->game.world,
                              &s->panel_bounds[PANEL_RIGHT],
                              font)) {
    return false;
  }

  if (!rl_alloc_character_view(&s->views[RL_VIEW_CHARACTER],
                               &s->run->game.world,
                               &s->panel_bounds[PANEL_BOTTOM],
                               font)) {
    return false;
  }

  if (!rl_alloc_log_view(&s->views[RL_VIEW_LOG],
                         &s->log,
                         &s->panel_bounds[PANEL_BOTTOM],
                         (float)font->tile_height)) {
    return false;
  }

  if (!rl_alloc_inv_view(&s->views[RL_VIEW_INVENTORY],
                         &s->run->game.world,
                         &s->panel_bounds[PANEL_BOTTOM],
                         font)) {
    return false;
  }

  // the world view is only designed to work in the main panel right now
  if (!rl_alloc_world_view(&s->views[RL_VIEW_WORLD],
                           &s->run->game.world,
                           &s->panel_bounds[PANEL_MAIN],
                           font->tile_width,
                           font->tile_height)) {
    return false;
  }

  if (!rl_init_game_log(&s->log)) {
    return false;
  }

  if (!alist_alloc(&s->events, 8)) {
    SDL_Log("alist_alloc failed: %s", SDL_GetError());
    return false;
  }

  s->repeat_cooldown = 0.0f;
  s->font = font;

  s->focused_panel = PANEL_MAIN;

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

  alist_free(&s->events);
  rl_free_game_log(&s->log);
  grid_free(&s->border_grid);
  SDL_free(s);
}

static void
cancel_focus(struct screen_state* s)
{
  s->focused_panel = PANEL_MAIN;
  s->panel_views[PANEL_BOTTOM] = RL_VIEW_LOG;
}

static void
enter_screen(void* data)
{
  struct screen_state* s = (struct screen_state*)data;
  SDL_assert(s != NULL);

  cancel_focus(s);
  s->panel_views[PANEL_MAIN] = RL_VIEW_WORLD;
  s->panel_views[PANEL_RIGHT] = RL_VIEW_IN_SIGHT;
  s->pending_target_cmd = (struct rl_command){ 0 };
  s->game_over = false;
  s->repeat_cooldown = 0.0f;

  alist_clear(&s->events);
  alist_clear(&s->log.messages);
}

static void
exit_screen(void* data)
{
  (void)data;
}

static void
toggle_bottom_view(struct screen_state* s, enum rl_view_id view)
{
  if (s->focused_panel == PANEL_BOTTOM &&
      s->panel_views[PANEL_BOTTOM] == view) {
    cancel_focus(s);
    return;
  }

  s->panel_views[PANEL_BOTTOM] = view;
  s->focused_panel = PANEL_BOTTOM;
}

static bool
rogue_is_dead(struct screen_state const* s)
{
  struct rl_world const* world = &s->run->game.world;
  struct rl_actor const* rogue = rl_borrow_actor(world, rl_get_rogue(world));

  return !rl_actor_is_alive(rogue);
}

static bool
rogue_has_won(struct screen_state const* s)
{
  return s->run->game.won && !rogue_is_dead(s);
}

static void
begin_game_over(struct screen_state* s)
{
  s->game_over = true;
  s->pending_target_cmd = (struct rl_command){ 0 };
  cancel_focus(s);

  enum rl_save_result const result = rl_save_run(s->run);
  if (result != RL_SAVE_OK) {
    SDL_Log("rl_save_run failed (%d): %s", (int)result, SDL_GetError());

    struct rl_text failure = { 0 };
    rl_append_text(&failure,
                   &RL_COLOUR_RED[4],
                   "Could not save the run; it will be retried on exit.");
    rl_log_text(&s->log, &failure);
  }

  struct rl_text prompt = { 0 };
  rl_append_text(&prompt,
                 NULL,
                 rogue_has_won(s) ? "You have slain the dragon! Press "
                                  : "You have died. Press ");
  rl_append_text(&prompt, &RL_COLOUR_YELLOW[3], "Q");
  rl_append_text(&prompt, NULL, " to exit.");
  rl_log_text(&s->log, &prompt);
}

static bool
submit_command(struct screen_state* s, struct rl_command const* cmd)
{
  alist_clear(&s->events);
  bool const handled = rl_update_game(&s->run->game, cmd, &s->events);

  // Consume this update's events exactly once, after submitting a command.
  for (int i = 0; i < alist_len(&s->events); i++) {
    struct rl_event const* event = alist_at(&s->events, i);
    rl_log_event(&s->log, event, &s->run->game.world);
  }

  if (rogue_is_dead(s) || rogue_has_won(s)) {
    begin_game_over(s);
  }

  return handled;
}

static bool
begin_target_select(struct screen_state* s,
                    handle(rl_item) item,
                    struct rl_item_consumable_def const* def)
{
  handle(rl_actor) const rogue_handle = rl_get_rogue(&s->run->game.world);
  struct rl_actor const* rogue =
    rl_borrow_actor(&s->run->game.world, rogue_handle);

  if (!rl_world_view_begin_select(&s->views[RL_VIEW_WORLD], rogue->pos, def)) {
    return false;
  }

  s->pending_target_cmd = (struct rl_command){ 0 };
  s->pending_target_cmd.actor = rogue_handle;
  s->pending_target_cmd.type = RL_COMMAND_USE_ITEM;
  s->pending_target_cmd.use_item.item = item;

  s->focused_panel = PANEL_MAIN;
  return true;
}

static void
resolve_pending_target(struct screen_state* s)
{
  SDL_Point dst;
  enum rl_world_selection_result result =
    rl_world_view_take_selection(&s->views[RL_VIEW_WORLD], &dst);

  switch (result) {
    case RL_WORLD_SELECTION_CONFIRMED:
      s->pending_target_cmd.use_item.dst = dst;
      submit_command(s, &s->pending_target_cmd);
      s->pending_target_cmd = (struct rl_command){ 0 };
      cancel_focus(s);
      break;
    case RL_WORLD_SELECTION_CANCELLED:
      s->pending_target_cmd = (struct rl_command){ 0 };
      cancel_focus(s);
      break;
    case RL_WORLD_SELECTION_NONE:
      break;
  }
}

static bool
handle_item_selection(struct screen_state* s, handle(rl_item) item_handle)
{
  struct rl_item const* item = rl_borrow_item(&s->run->game.world, item_handle);
  if (item == NULL) {
    return false;
  }

  struct rl_item_consumable_def const* def =
    rl_get_item_consumable_def(item->itype);
  if (def == NULL) {
    return false;
  }

  bool handled = false;

  switch (def->target) {
    case RL_ITEM_TARGET_NONE:
    case RL_ITEM_TARGET_CLOSEST: {
      // use item
      struct rl_command cmd = { 0 };
      cmd.actor = rl_get_rogue(&s->run->game.world);
      cmd.type = RL_COMMAND_USE_ITEM;
      cmd.use_item.item = item_handle;
      handled = submit_command(s, &cmd);
      cancel_focus(s);
      break;
    }
    case RL_ITEM_TARGET_TILE:
      handled = begin_target_select(s, item_handle, def);
      break;
  }

  return handled;
}

static bool
handle_action(struct screen_state* s, struct inpt_state const* istate)
{
  enum rl_view_id const view_id = s->panel_views[s->focused_panel];
  struct rl_view* view = &s->views[view_id];

  bool handled = rl_view_handle_input(view, istate);

  switch (view_id) {
    case RL_VIEW_INVENTORY: {
      handle(rl_item) const item = rl_inv_view_take_selection(view);
      handled = handle_item_selection(s, item) || handled;
      break;
    }
    case RL_VIEW_WORLD: {
      struct rl_command cmd;
      if (rl_world_view_take_command(view, &cmd)) {
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

static void
handle_input(struct screen_state* s, struct inpt_state const* istate)
{
  enum rl_action action = rl_handle_keyboard_input(istate);

  if (s->pending_target_cmd.type != RL_COMMAND_NONE) {
    // Targeting a tile, don't interrupt
    handle_action(s, istate);
  } else if (action == RL_ACTION_SHOW_INVENTORY) {
    toggle_bottom_view(s, RL_VIEW_INVENTORY);
  } else if (action == RL_ACTION_SHOW_LOG) {
    toggle_bottom_view(s, RL_VIEW_LOG);
  } else if (action == RL_ACTION_SHOW_CHARACTER) {
    toggle_bottom_view(s, RL_VIEW_CHARACTER);
  } else if (action == RL_ACTION_CANCEL) {
    if (!handle_action(s, istate)) {
      cancel_focus(s);
    }
  } else {
    handle_action(s, istate);
  }

  resolve_pending_target(s);
}

static struct rl_screen_transition
update_screen(void* data, struct inpt_state const* istate, float dt)
{
  struct rl_screen_transition transition = { 0 };
  struct screen_state* s = (struct screen_state*)data;
  SDL_assert(s != NULL);

  if (s->game_over) {
    // Only leaving is possible, and it must not wait on the key repeat timer.
    if (rl_handle_keyboard_input(istate) == RL_ACTION_CANCEL) {
      transition.type = RL_SCREEN_TRANSITION_SWAP;
      transition.target =
        rogue_has_won(s) ? RL_SCREEN_VICTORY : RL_SCREEN_GAME_OVER;
    }
  } else {
    s->repeat_cooldown = SDL_max(0.0f, s->repeat_cooldown - dt);
    if (s->repeat_cooldown <= 0.0f) {
      handle_input(s, istate);
    }
  }

  for (int panel = 0; panel < PANEL_COUNT; ++panel) {
    rl_prepare_view(&s->views[s->panel_views[panel]], dt);
  }

  return transition;
}

static void
describe_ribbon(void const* data, struct rl_ribbon_content* content)
{
  struct screen_state const* s = (struct screen_state const*)data;
  SDL_assert(s != NULL);

  if (s->game_over) {
    rl_append_text(&content->text[RL_RIBBON_LEFT],
                   &RL_COLOUR_CYAN[3],
                   rogue_has_won(s) ? "Victory" : "Game Over");
    rl_append_text(
      &content->text[RL_RIBBON_RIGHT], &RL_COLOUR_YELLOW[3], "[Q] exit");
    return;
  }

  rl_view_describe_ribbon(&s->views[s->panel_views[s->focused_panel]], content);
}

static void
render_screen(void const* data, SDL_Renderer* renderer)
{
  struct screen_state const* s = (struct screen_state*)data;
  SDL_assert(s != NULL);

  for (int panel = 0; panel < PANEL_COUNT; ++panel) {
    SDL_FRect const* bounds = &s->panel_bounds[panel];
    SDL_Rect clip = {
      (int)bounds->x,
      (int)bounds->y,
      (int)bounds->w,
      (int)bounds->h,
    };
    SDL_SetRenderClipRect(renderer, &clip);
    rl_render_view(&s->views[s->panel_views[panel]], renderer, s->font);
  }

  SDL_SetRenderClipRect(renderer, NULL);
  SDL_FPoint const origin = { s->frame_bounds.x, s->frame_bounds.y };
  rl_draw_cp437_borders(renderer,
                        s->font,
                        &s->border_grid,
                        RL_COLOUR_GRAY[5],
                        RL_COLOUR_BLACK,
                        origin);

  for (size_t tab = 0; tab < SDL_arraysize(BOTTOM_TABS); ++tab) {
    bool const selected = s->panel_views[PANEL_BOTTOM] == BOTTOM_TABS[tab].view;
    SDL_FColor const colour = selected ? RL_COLOUR_CYAN[3] : RL_COLOUR_GRAY[5];
    SDL_FPoint const at = {
      s->tab_bounds[tab].x + s->font->tile_width,
      s->tab_bounds[tab].y,
    };
    gfx_print_console(renderer,
                      s->font,
                      str_view_from_cstr(BOTTOM_TABS[tab].label),
                      colour,
                      RL_COLOUR_BLACK,
                      at);
  }
}

bool
rl_alloc_gameplay_screen(struct rl_screen* screen,
                         SDL_FRect const* bounds,
                         struct gfx_tileset const* font,
                         struct rl_run* run)
{
  SDL_assert(run != NULL);

  screen->state = SDL_calloc(1, sizeof(struct screen_state));
  if (screen->state == NULL) {
    return false;
  }

  if (!alloc_screen(screen->state, bounds, font, run)) {
    free_screen(screen->state);
    screen->state = NULL;
    return false;
  }

  screen->free = free_screen;
  screen->enter = enter_screen;
  screen->exit = exit_screen;
  screen->describe_ribbon = describe_ribbon;
  screen->update = update_screen;
  screen->render = render_screen;

  return true;
}
