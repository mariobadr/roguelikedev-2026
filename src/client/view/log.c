#include "log.h"

#include <SDL3/SDL_render.h>

#include "client/view/ribbon.h"

#include "client/game_log.h"
#include "client/palette.h"
#include "client/render.h"
#include "client/ui.h"

static int
current_first(struct rl_log_view const* view, struct rl_game_log const* log)
{
  int const count = (int)alist_len(&log->messages);

  if (view->follow_tail)
    return ui_list_max_offset(&view->list, count);

  return ui_list_offset(&view->list, count);
}

void
rl_init_log_view(struct rl_log_view* view,
                 SDL_FRect const* viewport,
                 float line_height)
{
  view->follow_tail = true;
  ui_list_init(&view->list,
               viewport,
               view->slots,
               SDL_arraysize(view->slots),
               line_height,
               2.0f);
}

void
rl_resize_log_view(struct rl_log_view* view, SDL_FRect const* viewport)
{
  ui_list_resize(&view->list, viewport);
}

void
rl_scroll_log_view_up(struct rl_log_view* view, struct rl_game_log const* log)
{
  rl_scroll_log_view_to(view, log, current_first(view, log) - 1);
}

void
rl_scroll_log_view_down(struct rl_log_view* view, struct rl_game_log const* log)
{
  rl_scroll_log_view_to(view, log, current_first(view, log) + 1);
}

void
rl_scroll_log_view_to(struct rl_log_view* view,
                      struct rl_game_log const* log,
                      int first)
{
  int const count = (int)alist_len(&log->messages);

  ui_list_scroll_to(&view->list, count, first);
  view->follow_tail =
    ui_list_offset(&view->list, count) == ui_list_max_offset(&view->list, count);
}

void
rl_log_view_ribbon(struct rl_log_view const* view, struct rl_ribbon* ribbon)
{
  (void)view;

  rl_set_current_view(ribbon, "Log");
  rl_set_current_mode(ribbon, "Scrolling");

  struct rl_text msg = { 0 };
  rl_append_text(&msg, &RL_COLOUR_YELLOW[3], "[WS]   ");
  rl_set_ribbon_text(ribbon, RL_RIBBON_RIGHT, &msg);
}

void
rl_draw_log_view(struct rl_log_view const* view,
                 SDL_Renderer* renderer,
                 struct rl_font const* font,
                 struct rl_game_log const* log)
{
  int const len = (int)alist_len(&log->messages);
  int const first = current_first(view, log);
  int const count = SDL_min(view->list.slot_count, len - first);

  for (int row = 0; row < count; ++row) {
    struct rl_text const* message = alist_at(&log->messages, first + row);

    SDL_FPoint const at = {
      view->slots[row].x,
      view->slots[row].y,
    };

    rl_draw_text(
      renderer, font, message, RL_COLOUR_GRAY[5], RL_COLOUR_BLACK, at);
  }
}
