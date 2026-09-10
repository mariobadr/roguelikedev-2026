#include "text.h"

#include <stdarg.h>

#include <SDL3/SDL_assert.h>

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
