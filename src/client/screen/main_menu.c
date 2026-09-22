#include "main_menu.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>

#include "game/level.h"

#include "graphics/console.h"
#include "save/save.h"
#include "ui/anchor.h"
#include "ui/list.h"

#include "render/palette.h"

#include "client/action.h"
#include "client/controls.h"
#include "client/font.h"
#include "client/ribbon.h"
#include "client/run.h"
#include "client/screen.h"
#include "client/text.h"

#define RL_NEW_GAME_WIDTH RL_MAX_MAP_WIDTH
#define RL_NEW_GAME_HEIGHT RL_MAX_MAP_HEIGHT
#define RL_SAVE_LIST_INITIAL_CAP 8

#define KEY_REPEAT_COOLDOWN (0.115f)

/**
 * From: https://patorjk.com/software/taag/#p=display&f=Graceful
 */
static char const* const GAME_OVER_BANNER[] = {
  "  __     ____   __    ___  _  _  ____  __    __  __ _  ____ ",
  " / _\\   (  _ \\ /  \\  / __)/ )( \\(  __)(  )  (  )(  / )(  __)",
  "/    \\   )   /(  O )( (_ \\) \\/ ( ) _) / (_/\\ )(  )  (  ) _) ",
  "\\_/\\_/  (__\\_) \\__/  \\___/\\____/(____)\\____/(__)(__\\_)(____)",
};

enum menu_item
{
  MENU_ITEM_CONTINUE,
  MENU_ITEM_NEW_GAME,
  MENU_ITEM_MANAGE_SAVES,
  MENU_ITEM_EXIT,
  MENU_ITEM_COUNT,
};

static char const* const MENU_LABELS[MENU_ITEM_COUNT] = {
  "Continue",
  "New Game",
  "Manage Saves",
  "Exit",
};

struct screen_state
{
  struct gfx_tileset const* font;

  SDL_FPoint banner_pos;

  struct ui_list_menu menu;
  SDL_FRect slots[MENU_ITEM_COUNT];
  float repeat_cooldown;

  struct rl_run* run;

  alist(rl_save_info) saves;
  struct rl_save_id continue_id;
};

static bool
is_enabled(void const* data, int item)
{
  struct screen_state const* s = data;
  if (item == MENU_ITEM_CONTINUE) {
    return rl_save_id_is_valid(s->continue_id);
  }
  return true;
}

static struct ui_list_menu_model
menu_model(struct screen_state const* s)
{
  return (struct ui_list_menu_model){
    .count = MENU_ITEM_COUNT,
    .is_enabled = is_enabled,
    .data = s,
  };
}

static void
refresh_saves(struct screen_state* s)
{
  if (!rl_list_saves(&s->saves)) {
    SDL_Log("rl_list_saves failed: %s", SDL_GetError());
  }

  s->continue_id = rl_save_id_invalid();
  for (size_t i = 0; i < alist_len(&s->saves); i++) {
    struct rl_save_info const* info = alist_at(&s->saves, i);
    if (rl_save_is_continuable(info)) {
      s->continue_id = info->id;
      break;
    }
  }

  ui_list_menu_sync(&s->menu, menu_model(s));
}

static void
measure_banner(struct gfx_tileset const* font, float* width, float* height)
{
  size_t cols = 0;
  for (size_t i = 0; i < SDL_arraysize(GAME_OVER_BANNER); i++) {
    cols = SDL_max(cols, SDL_strlen(GAME_OVER_BANNER[i]));
  }

  *width = (float)cols * (float)font->tile_width;
  *height = (float)SDL_arraysize(GAME_OVER_BANNER) * (float)font->tile_height;
}

static float
measure_menu_width(struct gfx_tileset const* font)
{
  size_t cols = 0;
  for (size_t i = 0; i < MENU_ITEM_COUNT; i++) {
    cols = SDL_max(cols, SDL_strlen(MENU_LABELS[i]));
  }

  return (float)cols * (float)font->tile_width;
}

static void
create_layout(SDL_FRect const* bounds,
              struct gfx_tileset const* font,
              float item_height,
              float gap,
              SDL_FPoint* banner_pos,
              SDL_FRect* list_viewport)
{
  float banner_width = 0.0f;
  float banner_height = 0.0f;
  measure_banner(font, &banner_width, &banner_height);
  float const banner_gap = 32.0f;

  float const list_width = measure_menu_width(font);
  float const list_height =
    MENU_ITEM_COUNT * item_height + (MENU_ITEM_COUNT - 1) * gap;

  float const block_width = SDL_max(banner_width, list_width);
  float const block_height = banner_height + banner_gap + list_height;

  struct ui_position const centre = { UI_ANCHOR_CENTRE, { 0.0f, 0.0f } };
  SDL_FRect const block = ui_resolve(centre, bounds, block_width, block_height);

  banner_pos->x = block.x + (block.w - banner_width) / 2.0f;
  banner_pos->y = block.y;

  *list_viewport = (SDL_FRect){
    block.x + (block.w - list_width) / 2.0f,
    block.y + banner_height + banner_gap,
    list_width,
    list_height,
  };
}

static bool
alloc_screen(struct screen_state* s,
             SDL_FRect const* bounds,
             struct gfx_tileset const* font,
             struct rl_run* run)
{
  s->font = font;
  s->menu.selected = -1;
  s->repeat_cooldown = 0.0f;
  s->run = run;

  float const item_height = (float)font->tile_height + 4.0f;
  float const gap = 4.0f;

  SDL_FRect list_viewport;
  create_layout(bounds, font, item_height, gap, &s->banner_pos, &list_viewport);

  ui_list_init(
    &s->menu.list, &list_viewport, s->slots, MENU_ITEM_COUNT, item_height, gap);

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

  refresh_saves(s);
}

static void
exit_screen(void* data)
{
  (void)data;
}

static bool
handle_run_result(struct rl_run_result result)
{
  switch (result.type) {
    case RL_RUN_RESULT_OK:
      return true;
    case RL_RUN_RESULT_GAME_ERROR:
      SDL_Log("Could not start a new game.");
      return false;
    case RL_RUN_RESULT_SAVE_ERROR:
      SDL_Log("Could not save/load the run.");
      return false;
    default:
      return false;
  }
}

static struct rl_screen_transition
select_item(struct screen_state* s, enum menu_item item)
{
  struct rl_screen_transition transition = { 0 };

  switch (item) {
    case MENU_ITEM_NEW_GAME: {
      Uint64 const seed = SDL_GetPerformanceCounter();
      struct rl_run_result const result =
        rl_start_run(s->run, RL_NEW_GAME_WIDTH, RL_NEW_GAME_HEIGHT, seed);
      if (handle_run_result(result)) {
        transition.type = RL_SCREEN_TRANSITION_PUSH;
        transition.target = RL_SCREEN_GAMEPLAY;
      }
      break;
    }
    case MENU_ITEM_CONTINUE: {
      struct rl_run_result const result = rl_resume_run(s->run, s->continue_id);
      if (handle_run_result(result)) {
        transition.type = RL_SCREEN_TRANSITION_PUSH;
        transition.target = RL_SCREEN_GAMEPLAY;
      }
      break;
    }
    case MENU_ITEM_MANAGE_SAVES:
      transition.type = RL_SCREEN_TRANSITION_PUSH;
      transition.target = RL_SCREEN_SAVE_FILES;
      break;
    case MENU_ITEM_EXIT:
      transition.type = RL_SCREEN_TRANSITION_POP;
      break;
    case MENU_ITEM_COUNT:
      break;
  }

  return transition;
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

  enum rl_action const action = rl_handle_keyboard_input(istate);
  switch (action) {
    case RL_ACTION_MOVE_UP:
      ui_list_menu_move(&s->menu, menu_model(s), -1);
      s->repeat_cooldown = KEY_REPEAT_COOLDOWN;
      break;
    case RL_ACTION_MOVE_DOWN:
      ui_list_menu_move(&s->menu, menu_model(s), +1);
      s->repeat_cooldown = KEY_REPEAT_COOLDOWN;
      break;
    case RL_ACTION_SELECT:
      if (ui_list_menu_select(&s->menu, menu_model(s), s->menu.selected)) {
        transition = select_item(s, (enum menu_item)s->menu.selected);
      }
      s->repeat_cooldown = KEY_REPEAT_COOLDOWN;
      break;
    default:
      break;
  }

  return transition;
}

static void
describe_ribbon(void const* data, struct rl_ribbon_content* content)
{
  (void)data;

  rl_append_text(
    &content->text[RL_RIBBON_LEFT], &RL_COLOUR_CYAN[3], "Main Menu");
  rl_append_text(&content->text[RL_RIBBON_RIGHT],
                 &RL_COLOUR_YELLOW[3],
                 "[WS] move   [E] select");
}

static void
render_screen(void const* data, SDL_Renderer* renderer)
{
  struct screen_state const* s = (struct screen_state const*)data;
  SDL_assert(s != NULL);

  // render the banner
  for (size_t i = 0; i < SDL_arraysize(GAME_OVER_BANNER); i++) {
    SDL_FPoint const at = {
      s->banner_pos.x,
      s->banner_pos.y + (float)i * (float)s->font->tile_height,
    };
    gfx_print_console(renderer,
                      s->font,
                      str_view_from_cstr(GAME_OVER_BANNER[i]),
                      RL_COLOUR_GRAY[5],
                      RL_COLOUR_BLACK,
                      at);
  }

  // render the menu
  for (int i = 0; i < MENU_ITEM_COUNT; ++i) {
    bool const enabled = is_enabled(s, i);
    bool const selected = enabled && i == s->menu.selected;

    struct rl_text text = { 0 };
    rl_append_text(&text, &RL_COLOUR_YELLOW[3], selected ? "> " : "  ");
    rl_append_text(&text, NULL, MENU_LABELS[i]);

    SDL_FColor const colour = enabled ? RL_COLOUR_GRAY[5] : RL_COLOUR_GRAY[7];
    SDL_FPoint const at = { s->slots[i].x, s->slots[i].y };
    rl_draw_text(renderer, s->font, &text, colour, RL_COLOUR_BLACK, at);
  }
}

bool
rl_alloc_main_menu_screen(struct rl_screen* screen,
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
