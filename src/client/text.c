#include "text.h"

#include <SDL3/SDL_stdinc.h>

#include "graphics/console.h"
#include "graphics/tileset.h"

static bool
append_span(struct rl_text* text, size_t start, size_t end, SDL_FColor colour)
{
  if (text->span_count > 0) {
    struct rl_text_span* last = &text->spans[text->span_count - 1];
    bool const adjacent = last->end == start;
    bool const same_colour =
      SDL_memcmp(&last->colour, &colour, sizeof colour) == 0;

    if (adjacent && same_colour) {
      // merge
      last->end = end;
      return true;
    }
  }

  if (text->span_count >= RL_TEXT_SPAN_CAPACITY) {
    // can't be merged and we are at capacity
    return false;
  }

  struct rl_text_span* span = &text->spans[text->span_count++];
  span->start = start;
  span->end = end;
  span->colour = colour;

  return true;
}

bool
rl_append_text(struct rl_text* text, SDL_FColor const* colour, char const* str)
{
  size_t const str_len = SDL_strlen(str);
  if (str_len == 0) {
    // nothing to copy
    return true;
  }

  size_t const available = RL_TEXT_CAPACITY - 1 - text->length;
  if (str_len > available) {
    // not enough space for this string
    return false;
  }

  size_t const start = text->length;

  bool ok = true;
  if (colour != NULL) {
    // should work...
    ok = append_span(text, start, start + str_len, *colour);
  }

  if (ok) {
    SDL_memcpy(text->content + start, str, str_len);
    text->length += str_len;
    text->content[text->length] = '\0';
  }

  return ok;
}

bool
rl_append_text_format(struct rl_text* text,
                      SDL_FColor const* colour,
                      char const* fmt,
                      ...)
{
  char buffer[RL_TEXT_CAPACITY];

  va_list args;
  va_start(args, fmt);
  int const required = SDL_vsnprintf(buffer, sizeof(buffer), fmt, args);
  va_end(args);

  if (required < 0) {
    // error in vsnprintf
    return false;
  }

  if (required >= sizeof(buffer)) {
    // ended up needing more bytes than we can handle
    return false;
  }

  return rl_append_text(text, colour, buffer);
}

void
rl_truncate_text(struct rl_text* text, size_t length)
{
  if (text->length <= length) {
    return;
  }

  text->length = length;
  text->content[length] = '\0';

  size_t kept = 0;
  for (size_t i = 0; i < text->span_count; ++i) {
    struct rl_text_span span = text->spans[i];
    if (span.start >= length) {
      break;
    }
    span.end = SDL_min(span.end, length);
    text->spans[kept++] = span;
  }
  text->span_count = kept;
}

static void
draw_text_range(SDL_Renderer* renderer,
                struct gfx_tileset const* font,
                struct rl_text const* text,
                size_t start,
                size_t end,
                SDL_FColor fg,
                SDL_FColor bg,
                SDL_FPoint at)
{
  struct str_view const view = { text->content + start, (int)(end - start) };
  SDL_FPoint const pos = { at.x + (float)start * font->tile_width, at.y };
  gfx_print_console(renderer, font, view, fg, bg, pos);
}

void
rl_draw_text(SDL_Renderer* renderer,
             struct gfx_tileset const* font,
             struct rl_text const* text,
             SDL_FColor fg,
             SDL_FColor bg,
             SDL_FPoint at)
{
  size_t cursor = 0;
  for (size_t i = 0; i < text->span_count; ++i) {
    struct rl_text_span const* span = &text->spans[i];

    if (cursor < span->start) {
      draw_text_range(renderer, font, text, cursor, span->start, fg, bg, at);
    }

    draw_text_range(
      renderer, font, text, span->start, span->end, span->colour, bg, at);
    cursor = span->end;
  }

  if (cursor < text->length) {
    draw_text_range(renderer, font, text, cursor, text->length, fg, bg, at);
  }
}
