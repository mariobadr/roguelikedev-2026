/**
 * @file log.h
 */
#ifndef GINC_ROGUELIKE_LOG_VIEW_H
#define GINC_ROGUELIKE_LOG_VIEW_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// forward declarations
struct rl_game_log;
struct rl_view;

bool
rl_alloc_log_view(struct rl_view* view,
                  struct rl_game_log const* log,
                  SDL_FRect const* viewport,
                  float line_height);

#endif // GINC_ROGUELIKE_LOG_VIEW_H
