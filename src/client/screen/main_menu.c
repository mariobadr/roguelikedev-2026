#include "client/screen.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_log.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_timer.h>

#include "ui/anchor.h"
#include "ui/list.h"

#include "client/action.h"
#include "client/controls.h"
#include "client/font.h"
#include "client/palette.h"
#include "client/render.h"
#include "client/run.h"
#include "client/save.h"
#include "client/text.h"
#include "client/ui.h"

#define RL_NEW_GAME_WIDTH 64
#define RL_NEW_GAME_HEIGHT 64
#define RL_SAVE_LIST_INITIAL_CAP 8

#define KEY_REPEAT_COOLDOWN (0.115f)

enum menu_item
{
  MENU_ITEM_CONTINUE,
  MENU_ITEM_NEW_GAME,
  MENU_ITEM_EXIT,
  MENU_ITEM_COUNT,
};

static char const* const MENU_LABELS[MENU_ITEM_COUNT] = {
  "Continue",
  "New Game",
  "Exit",
};

struct screen_state
{
  struct gfx_tileset const* font;

  struct ui_list list;
  SDL_FRect slots[MENU_ITEM_COUNT];
  int selected;
  float repeat_cooldown;

  struct rl_run* run;

  alist(rl_save_info) saves;
  struct rl_save_id continue_id;
};

static bool
is_enabled(struct screen_state const* s, enum menu_item item)
{
  if (item == MENU_ITEM_CONTINUE) {
    return rl_save_id_is_valid(s->continue_id);
  }
  return true;
}

static void
refresh_saves(struct screen_state* s)
{
  if (!rl_list_saves(&s->saves)) {
    SDL_Log("rl_list_saves failed: %s", SDL_GetError());
    alist_clear(&s->saves);
  }

  s->continue_id = rl_save_id_invalid();
  for (size_t i = 0; i < alist_len(&s->saves); i++) {
    struct rl_save_info const* info = alist_at(&s->saves, i);
    if (rl_save_is_continuable(info)) {
      s->continue_id = info->id;
      break;
    }
  }

  if (!is_enabled(s, (enum menu_item)s->selected)) {
    s->selected = MENU_ITEM_NEW_GAME;
  }
}

static bool
alloc_screen(struct screen_state* s,
             struct gfx_tileset const* font,
             struct rl_run* run)
{
  s->font = font;
  s->selected = 0;
  s->repeat_cooldown = 0.0f;
  s->run = run;

  float const item_height = (float)font->tile_height + 4.0f;
  float const gap = 4.0f;
  float const width = 160.0f;
  float const height =
    MENU_ITEM_COUNT * item_height + (MENU_ITEM_COUNT - 1) * gap;

  SDL_FRect const screen = {
    0.0f, 0.0f, (float)RL_UI_WIDTH, (float)RL_UI_HEIGHT
  };
  struct ui_position const centre = { UI_ANCHOR_CENTRE, { 0.0f, 0.0f } };
  SDL_FRect const viewport = ui_resolve(centre, &screen, width, height);

  ui_list_init(
    &s->list, &viewport, s->slots, MENU_ITEM_COUNT, item_height, gap);

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
      if (!rl_save_id_is_valid(s->continue_id)) {
        break;
      }
      struct rl_run_result const result = rl_resume_run(s->run, s->continue_id);
      if (handle_run_result(result)) {
        transition.type = RL_SCREEN_TRANSITION_PUSH;
        transition.target = RL_SCREEN_GAMEPLAY;
      }
      break;
    }
    case MENU_ITEM_EXIT:
      transition.type = RL_SCREEN_TRANSITION_POP;
      break;
    default:
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
    case RL_ACTION_MOVE_UP: {
      int candidate = s->selected - 1;
      while (candidate >= 0 && !is_enabled(s, (enum menu_item)candidate)) {
        candidate--;
      }
      if (candidate >= 0) {
        s->selected = candidate;
      }
      s->repeat_cooldown = KEY_REPEAT_COOLDOWN;
      break;
    }
    case RL_ACTION_MOVE_DOWN: {
      int candidate = s->selected + 1;
      while (candidate < MENU_ITEM_COUNT &&
             !is_enabled(s, (enum menu_item)candidate)) {
        candidate++;
      }
      if (candidate < MENU_ITEM_COUNT) {
        s->selected = candidate;
      }
      s->repeat_cooldown = KEY_REPEAT_COOLDOWN;
      break;
    }
    case RL_ACTION_SELECT:
      if (is_enabled(s, (enum menu_item)s->selected)) {
        transition = select_item(s, (enum menu_item)s->selected);
      }
      s->repeat_cooldown = KEY_REPEAT_COOLDOWN;
      break;
    default:
      break;
  }

  return transition;
}

static void
render_screen(void const* data, SDL_Renderer* renderer)
{
  struct screen_state const* s = (struct screen_state const*)data;
  SDL_assert(s != NULL);

  for (int i = 0; i < MENU_ITEM_COUNT; ++i) {
    bool const enabled = is_enabled(s, (enum menu_item)i);
    bool const selected = enabled && i == s->selected;

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
                          struct gfx_tileset const* font,
                          struct rl_run* run)
{
  SDL_assert(run != NULL);

  screen->state = SDL_calloc(1, sizeof(struct screen_state));
  if (screen->state == NULL) {
    return false;
  }

  if (!alloc_screen(screen->state, font, run)) {
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
