/**
 * @file log.h
 */
#ifndef GINC_ROGUELIKE_LOG_VIEW_H
#define GINC_ROGUELIKE_LOG_VIEW_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct rl_font;
struct rl_game_log;
struct rl_ribbon;

struct rl_log_view
{
  /** First visible message when not following the tail. */
  int first;
  /** Keep the latest messages visible. */
  bool follow_tail;
  SDL_FRect viewport;
  float line_height;
  int slot_count;
  SDL_FRect slots[8]; // temporary; these are the rects where we draw the text
};

void
rl_init_log_view(struct rl_log_view* view,
                 SDL_FRect const* viewport,
                 float line_height);

void
rl_resize_log_view(struct rl_log_view* view, SDL_FRect const* viewport);

void
rl_scroll_log_view_up(struct rl_log_view* view, struct rl_game_log const* log);

void
rl_scroll_log_view_down(struct rl_log_view* view,
                        struct rl_game_log const* log);

void
rl_scroll_log_view_to(struct rl_log_view* view,
                      struct rl_game_log const* log,
                      int first);

/**
 * Update ribbon based on the log view's current state.
 */
void
rl_log_view_ribbon(struct rl_log_view const* view, struct rl_ribbon* ribbon);

void
rl_draw_log_view(struct rl_log_view const* view,
                 SDL_Renderer* renderer,
                 struct rl_font const* font,
                 struct rl_game_log const* log);

#endif // GINC_ROGUELIKE_LOG_VIEW_H
