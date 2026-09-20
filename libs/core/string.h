/**
 * @file string.h
 */
#ifndef GINC_CORE_STRING_H
#define GINC_CORE_STRING_H

#include <SDL3/SDL_stdinc.h>

/**
 * A view of contiguous bytes, which need not be null-terminated.
 */
struct str_view
{
  /** Pointer to the first byte of the view. */
  char const* data;
  /** Number of bytes in the view; must be nonnegative. */
  int length;
};

/**
 * @return a view of the null-terminated str, excluding the terminator.
 */
static inline struct str_view
str_view_from_cstr(char const* str)
{
  return (struct str_view){ str, (int)SDL_strlen(str) };
}

#endif // GINC_CORE_STRING_H
