/**
 * @file log.h
 */
#ifndef GINC_ROGUELIKE_LOG_H
#define GINC_ROGUELIKE_LOG_H

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_stdinc.h>

#include "alist.h"

// forward declarations
struct rl_world;
struct rl_event_attack;
struct rl_event_death;

struct rl_log_char
{
  Uint8 glyph;
  SDL_FColor fg;
};

struct rl_log_line
{
  struct rl_log_char msg[64];
  int len;
};

alist_define_as(struct rl_log_line, rl_log_line);

struct rl_log_line
rl_build_attack_log(struct rl_world const* world,
                    struct rl_event_attack const* event);

struct rl_log_line
rl_build_death_log(struct rl_world const* world,
                   struct rl_event_death const* event);

#endif // GINC_ROGUELIKE_LOG_H