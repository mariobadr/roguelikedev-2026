/**
 * @file text.h
 */
#ifndef GINC_ROGUELIKE_TEXT_H
#define GINC_ROGUELIKE_TEXT_H

#include <SDL3/SDL_pixels.h>
#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct gfx_tileset;

/** Bytes of content, including the null terminator. */
#define RL_TEXT_CAPACITY 256
/** Maximum number of colour spans. */
#define RL_TEXT_SPAN_CAPACITY 8

/**
 * A range of text drawn in a colour.
 */
struct rl_text_span
{
  /** Index of the first byte in the span. */
  size_t start;
  /** Index one past the last byte in the span. */
  size_t end;
  /** The colour of the span. */
  SDL_FColor colour;
};

/**
 * Coloured text.
 */
struct rl_text
{
  /** The bytes of text, null-terminated. */
  char content[RL_TEXT_CAPACITY];
  /** The number of bytes in content, excluding the null terminator. */
  size_t length;
  /** The colour spans, in order. */
  struct rl_text_span spans[RL_TEXT_SPAN_CAPACITY];
  /** The number of spans in use. */
  size_t span_count;
};

/**
 * Append str to text, coloured by colour if it is not NULL.
 *
 * @return whether str was appended.
 */
bool
rl_append_text(struct rl_text* text, SDL_FColor const* colour, char const* str);

/**
 * Append a formatted string to text, coloured by colour if it is not NULL.
 *
 * @return whether the string was appended.
 */
bool
rl_append_text_format(struct rl_text* text,
                      SDL_FColor const* colour,
                      SDL_PRINTF_FORMAT_STRING char const* fmt,
                      ...) SDL_PRINTF_VARARG_FUNC(3);

/**
 * Draw text, using fg for any content not covered by a colour span.
 */
void
rl_draw_text(SDL_Renderer* renderer,
             struct gfx_tileset const* font,
             struct rl_text const* text,
             SDL_FColor fg,
             SDL_FColor bg,
             SDL_FPoint at);

#endif // GINC_ROGUELIKE_TEXT_H
