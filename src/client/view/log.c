#include "log.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_render.h>

#include "input/input.h"

#include "ui/list.h"

#include "client/view/ribbon.h"

#include "client/action.h"
#include "client/controls.h"
#include "client/game_log.h"
#include "client/palette.h"
#include "client/render.h"
#include "client/ui.h"
#include "client/view.h"

struct view_state
{
  struct rl_game_log const* log;
  struct ui_list list;
  /** Keep the latest messages visible. */
  bool follow_tail;
  SDL_FRect slots[8]; // temporary; these are the rects where we draw the text
};

static int
first_visible_index(struct view_state const* s)
{
  int const count = (int)alist_len(&s->log->messages);

  if (s->follow_tail) {
    return ui_list_max_offset(&s->list, count);
  }

  return ui_list_offset(&s->list, count);
}

static void
scroll_to(struct view_state* s, int first)
{
  int const count = (int)alist_len(&s->log->messages);

  ui_list_scroll_to(&s->list, count, first);
  s->follow_tail =
    ui_list_offset(&s->list, count) == ui_list_max_offset(&s->list, count);
}

static void
init_view_state(struct view_state* s,
                struct rl_game_log const* log,
                SDL_FRect const* viewport,
                float line_height)
{
  s->follow_tail = true;
  s->log = log;

  ui_list_init(
    &s->list, viewport, s->slots, SDL_arraysize(s->slots), line_height, 2.0f);
}

static void
update_ribbon(void const* data, struct rl_ribbon* ribbon)
{
  (void)data;

  rl_set_current_view(ribbon, "Log");
  rl_set_current_mode(ribbon, "Scrolling");

  struct rl_text msg = { 0 };
  rl_append_text(&msg, &RL_COLOUR_YELLOW[3], "[WS]");
  rl_set_ribbon_text(ribbon, RL_RIBBON_RIGHT, &msg);
}

static bool
update_view(void* data, struct inpt_state const* istate, struct rl_command* out)
{
  (void)out;

  struct view_state* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  enum rl_action const action = rl_handle_keyboard_input(istate);
  switch (action) {
    case RL_ACTION_MOVE_UP:
      scroll_to(s, first_visible_index(s) - 1);
      return true;
    case RL_ACTION_MOVE_DOWN:
      scroll_to(s, first_visible_index(s) + 1);
      return true;
    default:
      break;
  }

  return false;
}

static void
prepare_view(void* data)
{
  (void)data;
}

static void
render_view(void const* data,
            SDL_Renderer* renderer,
            struct rl_font const* font)
{
  struct view_state const* s = (struct view_state*)data;
  SDL_assert(s != NULL);

  int const len = (int)alist_len(&s->log->messages);
  int const first = first_visible_index(s);
  int const count = SDL_min(s->list.slot_count, len - first);

  for (int row = 0; row < count; ++row) {
    struct rl_text const* message = alist_at(&s->log->messages, first + row);

    SDL_FPoint const at = {
      s->slots[row].x,
      s->slots[row].y,
    };

    rl_draw_text(
      renderer, font, message, RL_COLOUR_GRAY[5], RL_COLOUR_BLACK, at);
  }
}

bool
rl_alloc_log_view(struct rl_view* view,
                  struct rl_game_log const* log,
                  SDL_FRect const* viewport,
                  float line_height)
{
  view->state = SDL_calloc(1, sizeof(struct view_state));
  if (view->state == NULL) {
    return false;
  }

  init_view_state(view->state, log, viewport, line_height);

  view->free = SDL_free;
  view->update_ribbon = update_ribbon;
  view->update = update_view;
  view->prepare = prepare_view;
  view->render = render_view;

  return true;
}
