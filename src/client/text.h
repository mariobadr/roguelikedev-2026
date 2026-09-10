/**
 * @file text.h
 */
#ifndef GINC_ROGUELIKE_TEXT_H
#define GINC_ROGUELIKE_TEXT_H

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_stdinc.h>

/** Bytes of content, including the null terminator. */
#define RL_TEXT_CAPACITY 256
/** Maximum number of colour spans. */
#define RL_TEXT_SPAN_CAPACITY 8

struct rl_text_span
{
  size_t start;
  size_t end;
  SDL_FColor colour;
};

/**
 * Coloured text.
 */
struct rl_text
{
  char content[RL_TEXT_CAPACITY];
  size_t length;
  struct rl_text_span spans[RL_TEXT_SPAN_CAPACITY];
  size_t span_count;
};

bool
rl_append_text(struct rl_text* text, SDL_FColor const* colour, char const* str);

bool
rl_append_text_format(struct rl_text* text,
                      SDL_FColor const* colour,
                      SDL_PRINTF_FORMAT_STRING char const* fmt,
                      ...) SDL_PRINTF_VARARG_FUNC(3);

#endif // GINC_ROGUELIKE_TEXT_H
