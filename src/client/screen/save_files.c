#include "save_files.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_time.h>

#include "graphics/tileset.h"
#include "input/input.h"

#include "ui/list.h"
#include "ui/rectcut.h"

#include "client/palette.h"
#include "client/render.h"
#include "client/ribbon.h"
#include "client/run.h"
#include "client/save.h"
#include "client/screen.h"
#include "client/text.h"

#define RL_SAVE_VISIBLE_ROWS 16
#define RL_SAVE_LIST_INITIAL_CAP 8
#define RL_SAVE_DETAIL_COLUMNS 32
#define RL_SAVE_DETAIL_LABEL_COLUMNS 12

#define KEY_REPEAT_COOLDOWN (0.115f)

enum screen_mode
{
  MODE_BROWSE,  //< choose a save
  MODE_CONFIRM, //< confirm deletion
};

enum confirm_item
{
  CONFIRM_ITEM_DELETE,
  CONFIRM_ITEM_CANCEL,
  CONFIRM_ITEM_COUNT,
};

static char const* const CONFIRM_LABELS[CONFIRM_ITEM_COUNT] = {
  "Delete",
  "Cancel",
};

struct screen_state
{
  struct gfx_tileset const* font;

  SDL_FPoint summary_pos;
  SDL_FRect detail_bounds;
  SDL_FPoint status_pos;

  enum screen_mode mode;
  float repeat_cooldown;

  struct ui_list_menu saves_menu;
  SDL_FRect save_slots[RL_SAVE_VISIBLE_ROWS];

  struct ui_list_menu confirm_menu;
  SDL_FRect confirm_slots[CONFIRM_ITEM_COUNT];

  struct rl_run* run;

  alist(rl_save_info) saves;

  struct rl_text status;
};

static struct ui_list_menu_model
saves_model(struct screen_state const* s)
{
  return (struct ui_list_menu_model){
    .count = (int)alist_len(&s->saves),
    .is_enabled = NULL,
    .data = s,
  };
}

static struct rl_save_info const*
selected_save(struct screen_state const* s)
{
  int const selected = s->saves_menu.selected;
  if (selected < 0 || (size_t)selected >= alist_len(&s->saves)) {
    return NULL;
  }

  return alist_at(&s->saves, (size_t)selected);
}

static struct ui_list_menu_model
confirm_model(struct screen_state const* s)
{
  return (struct ui_list_menu_model){
    .count = CONFIRM_ITEM_COUNT,
    .is_enabled = NULL,
    .data = s,
  };
}

static void
set_status(struct screen_state* s,
           SDL_FColor const* colour,
           char const* message)
{
  s->status = (struct rl_text){ 0 };
  rl_append_text(&s->status, colour, message);
}

static char const*
run_error_message(struct rl_run_result result)
{
  switch (result.type) {
    case RL_RUN_RESULT_GAME_ERROR:
      return "Could not load the game.";
    case RL_RUN_RESULT_SAVE_ERROR:
      switch (result.save_error) {
        case RL_SAVE_MISSING:
          return "That save no longer exists.";
        case RL_SAVE_INCOMPATIBLE:
          return "That save was written by a different version.";
        case RL_SAVE_CORRUPT:
          return "That save is damaged and cannot be loaded.";
        case RL_SAVE_UNAVAILABLE:
          return "That run has already finished.";
        default:
          return "Could not read that save.";
      }
    default:
      return "Could not load that save.";
  }
}

static void
refresh_saves(struct screen_state* s)
{
  if (!rl_list_saves(&s->saves)) {
    SDL_Log("rl_list_saves failed: %s", SDL_GetError());
    alist_clear(&s->saves);
    set_status(s, &RL_COLOUR_RED[4], "Could not read the save folder.");
  }

  ui_list_menu_sync(&s->saves_menu, saves_model(s));
}

static char const*
outcome_label(enum rl_run_outcome outcome)
{
  switch (outcome) {
    case RL_RUN_ACTIVE:
      return "Active";
    case RL_RUN_DEAD:
      return "Dead";
    case RL_RUN_VICTORY:
      return "Victory";
    default:
      return "Unknown";
  }
}

static char const*
condition_label(enum rl_save_condition condition)
{
  switch (condition) {
    case RL_SAVE_CONDITION_OK:
      return "Readable";
    case RL_SAVE_CONDITION_INCOMPATIBLE:
      return "Incompatible";
    case RL_SAVE_CONDITION_CORRUPT:
      return "Corrupt";
    default:
      return "Unknown";
  }
}

static void
format_saved_at(SDL_Time saved_at, char* out, size_t size)
{
  SDL_DateTime dt;
  if (!SDL_TimeToDateTime(saved_at, &dt, true)) {
    SDL_strlcpy(out, "Unknown", size);
    return;
  }

  SDL_snprintf(out,
               size,
               "%04d-%02d-%02d %02d:%02d",
               dt.year,
               dt.month,
               dt.day,
               dt.hour,
               dt.minute);
}

static void
init_layout(struct screen_state* s,
            SDL_FRect const* bounds,
            float* item_height,
            float* gap,
            SDL_FRect* list_bounds)
{
  float const line = (float)s->font->tile_height;
  float const margin = 8.0f;

  SDL_FRect area = {
    bounds->x + margin,
    bounds->y + margin,
    bounds->w - 2.0f * margin,
    bounds->h - 2.0f * margin,
  };

  SDL_FRect const status = ui_cut_bottom(&area, line);
  ui_cut_bottom(&area, margin);

  float const detail_width =
    (float)(RL_SAVE_DETAIL_COLUMNS * s->font->tile_width);
  s->detail_bounds = ui_cut_right(&area, detail_width);
  ui_cut_right(&area, margin);

  s->status_pos = (SDL_FPoint){ status.x, status.y };
  s->summary_pos = (SDL_FPoint){ area.x, area.y };

  *item_height = line + 4.0f;
  *gap = 4.0f;
  *list_bounds = area;
}

static bool
alloc_screen(struct screen_state* s,
             SDL_FRect const* bounds,
             struct gfx_tileset const* font,
             struct rl_run* run)
{
  s->font = font;
  s->run = run;
  s->mode = MODE_BROWSE;
  s->saves_menu.selected = -1;
  s->confirm_menu.selected = -1;
  s->repeat_cooldown = 0.0f;

  float item_height = 0.0f;
  float gap = 0.0f;
  SDL_FRect list_bounds = { 0 };
  init_layout(s, bounds, &item_height, &gap, &list_bounds);

  ui_list_init(&s->saves_menu.list,
               &list_bounds,
               s->save_slots,
               RL_SAVE_VISIBLE_ROWS,
               item_height,
               gap);

  // the confirmation menu sits below the selected save's summary line
  SDL_FRect confirm_bounds = list_bounds;
  ui_cut_top(&confirm_bounds, item_height);
  ui_cut_top(&confirm_bounds, gap);
  ui_list_init(&s->confirm_menu.list,
               &confirm_bounds,
               s->confirm_slots,
               CONFIRM_ITEM_COUNT,
               item_height,
               gap);

  if (!alist_alloc(&s->saves, RL_SAVE_LIST_INITIAL_CAP)) {
    SDL_Log("alist_alloc failed: %s", SDL_GetError());
    return false;
  }

  return true;
}

static void
free_screen(void* data)
{
  struct screen_state* s = (struct screen_state*)data;
  if (s == NULL) {
    return;
  }

  alist_free(&s->saves);
  SDL_free(s);
}

static void
enter_screen(void* data)
{
  struct screen_state* s = (struct screen_state*)data;
  SDL_assert(s != NULL);

  s->mode = MODE_BROWSE;
  s->status = (struct rl_text){ 0 };
  s->repeat_cooldown = 0.0f;
  s->saves_menu.selected = 0;
  refresh_saves(s);
}

static void
exit_screen(void* data)
{
  (void)data;
}

static struct rl_screen_transition
load_selected_save(struct screen_state* s)
{
  struct rl_screen_transition transition = { 0 };

  struct rl_save_info const* info = selected_save(s);
  if (info == NULL || !rl_save_is_continuable(info)) {
    return transition;
  }

  struct rl_run_result const result = rl_resume_run(s->run, info->id);
  if (result.type == RL_RUN_RESULT_OK) {
    transition.type = RL_SCREEN_TRANSITION_SWAP;
    transition.target = RL_SCREEN_GAMEPLAY;
    return transition;
  }

  set_status(s, &RL_COLOUR_RED[4], run_error_message(result));
  s->mode = MODE_BROWSE;
  refresh_saves(s);

  return transition;
}

static void
open_confirm_delete(struct screen_state* s)
{
  if (selected_save(s) == NULL) {
    return;
  }

  s->mode = MODE_CONFIRM;
  s->repeat_cooldown = KEY_REPEAT_COOLDOWN;
  s->confirm_menu.selected = CONFIRM_ITEM_CANCEL;
  ui_list_menu_sync(&s->confirm_menu, confirm_model(s));
  set_status(s, &RL_COLOUR_RED[5], "Delete this save permanently?");
}

static void
delete_selected_save(struct screen_state* s)
{
  struct rl_save_info const* info = selected_save(s);
  if (info == NULL) {
    s->mode = MODE_BROWSE;
    return;
  }

  // the list is refreshed below, so keep the id rather than the pointer
  struct rl_save_id const id = info->id;

  if (rl_delete_save(id) == RL_SAVE_OK) {
    if (rl_save_id_equal(id, s->run->save_id)) {
      s->run->save_id = rl_save_id_invalid();
    }
    set_status(s, NULL, "Save deleted.");
  } else {
    set_status(s, &RL_COLOUR_RED[4], "Could not delete that save.");
  }

  s->mode = MODE_BROWSE;
  refresh_saves(s);
}

static struct rl_screen_transition
update_screen(void* data, struct inpt_state const* istate, float dt)
{
  struct rl_screen_transition transition = { 0 };
  struct screen_state* s = (struct screen_state*)data;
  SDL_assert(s != NULL);

  s->repeat_cooldown = SDL_max(0.0f, s->repeat_cooldown - dt);

  // Screen-local shortcuts are edge-triggered and independent of key repeat.
  if (inpt_was_pressed(istate->keys[SDL_SCANCODE_ESCAPE])) {
    if (s->mode == MODE_CONFIRM) {
      s->mode = MODE_BROWSE;
      s->status = (struct rl_text){ 0 };
    } else {
      transition.type = RL_SCREEN_TRANSITION_POP;
    }
    return transition;
  }

  if (s->mode == MODE_BROWSE) {
    if (inpt_was_pressed(istate->keys[SDL_SCANCODE_E])) {
      return load_selected_save(s);
    }
    if (inpt_was_pressed(istate->keys[SDL_SCANCODE_D])) {
      open_confirm_delete(s);
      return transition;
    }
  } else if (inpt_was_pressed(istate->keys[SDL_SCANCODE_E])) {
    if (s->confirm_menu.selected == CONFIRM_ITEM_DELETE) {
      delete_selected_save(s);
    } else {
      s->mode = MODE_BROWSE;
      s->status = (struct rl_text){ 0 };
    }
    s->repeat_cooldown = KEY_REPEAT_COOLDOWN;
    return transition;
  }

  // Only held navigation keys need throttling.
  if (s->repeat_cooldown > 0.0f) {
    return transition;
  }

  int direction = 0;
  if (inpt_is_down(istate->keys[SDL_SCANCODE_W])) {
    direction = -1;
  } else if (inpt_is_down(istate->keys[SDL_SCANCODE_S])) {
    direction = +1;
  }

  if (direction != 0) {
    if (s->mode == MODE_BROWSE) {
      ui_list_menu_move(&s->saves_menu, saves_model(s), direction);
    } else {
      ui_list_menu_move(&s->confirm_menu, confirm_model(s), direction);
    }
    s->repeat_cooldown = KEY_REPEAT_COOLDOWN;
  }

  return transition;
}

static void
render_save_row(struct screen_state const* s,
                SDL_Renderer* renderer,
                struct rl_save_info const* info,
                SDL_FRect const* slot,
                bool selected)
{
  struct rl_text text = { 0 };
  rl_append_text(&text, &RL_COLOUR_YELLOW[3], selected ? "> " : "  ");

  if (info->condition == RL_SAVE_CONDITION_OK) {
    char when[32];
    format_saved_at(info->saved_at, when, sizeof(when));
    rl_append_text_format(
      &text, NULL, "%s  %s", when, outcome_label(info->outcome));
  } else {
    rl_append_text_format(
      &text, NULL, "----------------  %s", condition_label(info->condition));
  }

  SDL_FPoint const at = { slot->x, slot->y };
  rl_draw_text(
    renderer, s->font, &text, RL_COLOUR_GRAY[5], RL_COLOUR_BLACK, at);
}

static void
render_detail_line(struct screen_state const* s,
                   SDL_Renderer* renderer,
                   int row,
                   char const* label,
                   struct rl_text const* value)
{
  struct rl_text text = { 0 };
  rl_append_text_format(
    &text, &RL_COLOUR_RED[3], "%-*s", RL_SAVE_DETAIL_LABEL_COLUMNS, label);
  rl_append_text(&text, NULL, value->content);

  SDL_FPoint const at = {
    s->detail_bounds.x,
    s->detail_bounds.y + (float)(row * s->font->tile_height),
  };
  rl_draw_text(
    renderer, s->font, &text, RL_COLOUR_GRAY[2], RL_COLOUR_BLACK, at);
}

static void
render_detail(struct screen_state const* s,
              SDL_Renderer* renderer,
              struct rl_save_info const* info)
{
  struct rl_text value;
  int row = 0;

  if (info->condition == RL_SAVE_CONDITION_OK) {
    char when[32];
    format_saved_at(info->saved_at, when, sizeof(when));

    value = (struct rl_text){ 0 };
    rl_append_text(&value, NULL, when);
    render_detail_line(s, renderer, row++, "Last saved", &value);

    value = (struct rl_text){ 0 };
    rl_append_text(&value, NULL, outcome_label(info->outcome));
    render_detail_line(s, renderer, row++, "Outcome", &value);

    value = (struct rl_text){ 0 };
    rl_append_text_format(&value, NULL, "%d", info->depth);
    render_detail_line(s, renderer, row++, "Depth", &value);

    value = (struct rl_text){ 0 };
    rl_append_text_format(&value, NULL, "%" SDL_PRIu64, info->turns);
    render_detail_line(s, renderer, row++, "Turns", &value);

    value = (struct rl_text){ 0 };
    rl_append_text_format(&value, NULL, "%d/%d", info->hp, info->max_hp);
    render_detail_line(s, renderer, row++, "HP", &value);
  } else {
    value = (struct rl_text){ 0 };
    rl_append_text(&value, NULL, condition_label(info->condition));
    render_detail_line(s, renderer, row++, "Condition", &value);

    if (info->condition == RL_SAVE_CONDITION_INCOMPATIBLE) {
      value = (struct rl_text){ 0 };
      rl_append_text_format(&value, NULL, "%" SDL_PRIu32, info->version);
      render_detail_line(s, renderer, row++, "Version", &value);
    }
  }

  value = (struct rl_text){ 0 };
  rl_append_text_format(&value, NULL, "%016" SDL_PRIx64, info->id.value);
  render_detail_line(s, renderer, row, "Save", &value);
}

static void
render_save_list(struct screen_state const* s, SDL_Renderer* renderer)
{
  int const count = (int)alist_len(&s->saves);
  if (count == 0) {
    SDL_FPoint const at = {
      s->saves_menu.list.bounds.x,
      s->saves_menu.list.bounds.y,
    };
    rl_draw_string(renderer,
                   s->font,
                   "No saved runs.",
                   RL_COLOUR_GRAY[3],
                   RL_COLOUR_BLACK,
                   at);
    return;
  }

  int const offset = ui_list_offset(&s->saves_menu.list, count);
  int const end = ui_list_end(&s->saves_menu.list, count);
  for (int i = offset; i < end; ++i) {
    struct rl_save_info const* info = alist_at(&s->saves, (size_t)i);
    render_save_row(s,
                    renderer,
                    info,
                    &s->save_slots[i - offset],
                    i == s->saves_menu.selected);
  }
}

static void
render_confirm_delete(struct screen_state const* s,
                      SDL_Renderer* renderer,
                      struct rl_save_info const* info)
{
  SDL_FRect const summary_slot = { s->summary_pos.x, s->summary_pos.y, 0, 0 };
  render_save_row(s, renderer, info, &summary_slot, false);

  for (int i = 0; i < CONFIRM_ITEM_COUNT; ++i) {
    bool const selected = i == s->confirm_menu.selected;

    struct rl_text text = { 0 };
    rl_append_text(&text, &RL_COLOUR_YELLOW[3], selected ? "> " : "  ");
    rl_append_text(&text, NULL, CONFIRM_LABELS[i]);

    SDL_FPoint const at = { s->confirm_slots[i].x, s->confirm_slots[i].y };
    rl_draw_text(
      renderer, s->font, &text, RL_COLOUR_GRAY[5], RL_COLOUR_BLACK, at);
  }
}

static void
describe_ribbon(void const* data, struct rl_ribbon_content* content)
{
  struct screen_state const* s = (struct screen_state const*)data;
  SDL_assert(s != NULL);

  rl_append_text(
    &content->text[RL_RIBBON_LEFT], &RL_COLOUR_CYAN[3], "Manage Saves");

  SDL_FColor const* const enabled = &RL_COLOUR_YELLOW[3];
  SDL_FColor const* const disabled = &RL_COLOUR_GRAY[7];

  struct rl_text* hint = &content->text[RL_RIBBON_RIGHT];
  rl_append_text(hint, enabled, "[WS] move   ");
  if (s->mode == MODE_CONFIRM) {
    rl_append_text(hint, enabled, "[E] select   [Esc] cancel");
  } else {
    struct rl_save_info const* info = selected_save(s);
    bool const can_load = info != NULL && rl_save_is_continuable(info);
    rl_append_text(hint, can_load ? enabled : disabled, "[E] load");
    rl_append_text(hint, enabled, "   ");
    rl_append_text(hint, info != NULL ? enabled : disabled, "[D] delete");
    rl_append_text(hint, enabled, "   [Esc] back");
  }
}

static void
render_screen(void const* data, SDL_Renderer* renderer)
{
  struct screen_state const* s = (struct screen_state const*)data;
  SDL_assert(s != NULL);

  struct rl_save_info const* selected = selected_save(s);
  if (s->mode == MODE_CONFIRM && selected != NULL) {
    render_confirm_delete(s, renderer, selected);
  } else {
    render_save_list(s, renderer);
  }

  // render the details of the selected save
  if (selected != NULL) {
    render_detail(s, renderer, selected);
  }

  rl_draw_text(renderer,
               s->font,
               &s->status,
               RL_COLOUR_GRAY[2],
               RL_COLOUR_BLACK,
               s->status_pos);
}

bool
rl_alloc_save_files_screen(struct rl_screen* screen,
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
