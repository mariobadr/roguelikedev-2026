#include "log.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_render.h>

#include "ui/layout.h"

#include "client/view/ribbon.h"

#include "client/game_log.h"
#include "client/palette.h"
#include "client/render.h"
#include "client/ui.h"

static int
last_first(struct rl_log_view const* view, struct rl_game_log const* log)
{
  int const len = (int)alist_len(&log->messages);
  return SDL_max(0, len - view->slot_count);
}

static int
current_first(struct rl_log_view const* view, struct rl_game_log const* log)
{
  int const last = last_first(view, log);

  if (view->follow_tail)
    return last;

  return SDL_clamp(view->first, 0, last);
}

void
rl_init_log_view(struct rl_log_view* view,
                 SDL_FRect const* viewport,
                 float line_height)
{
  SDL_assert(line_height > 0);

  view->first = 0;
  view->follow_tail = true;
  view->line_height = line_height;
  rl_resize_log_view(view, viewport);
}

void
rl_resize_log_view(struct rl_log_view* view, SDL_FRect const* viewport)
{
  view->viewport = *viewport;

  struct ui_strip strip = { 0 };
  strip.item_width = viewport->w;
  strip.item_height = view->line_height;
  strip.gap = 2.0f;

  float const capacity =
    SDL_floorf((viewport->h + strip.gap) / (strip.item_height + strip.gap));

  view->slot_count = (int)SDL_min(capacity, SDL_arraysize(view->slots));
  strip.count = view->slot_count;

  struct ui_position const pos = { .anchor = UI_ANCHOR_TOP_LEFT };
  ui_layout_column(pos, viewport, strip, view->slots);
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
  int const last = last_first(view, log);

  view->first = SDL_clamp(first, 0, last);
  view->follow_tail = view->first == last;
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
  int const count = SDL_min(view->slot_count, len - first);

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
