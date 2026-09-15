/**
 * @file handle.h
 */
#ifndef GINC_CORE_HANDLE_H
#define GINC_CORE_HANDLE_H

#include <SDL3/SDL_stdinc.h>

/**
 * Produces the struct tag name for a given tag identifier. Primarily for
 * internal use.
 */
#define handle_tag(tag) tag##_handle

/**
 * Refers to a handle type by its tag. The tag must match one previously
 * defined with handle_define.
 */
#define handle(tag) struct handle_tag(tag)

/**
 * Defines a distinct handle type with an index and generation.
 * Generation 0 denotes an invalid handle.
 */
#define handle_define(tag)                                                     \
  struct handle_tag(tag)                                                       \
  {                                                                            \
    Uint32 index;                                                              \
    Uint32 generation;                                                         \
  }

/**
 * @return an invalid handle of the given type.
 */
#define handle_invalid(tag) ((handle(tag)){ 0, 0 })

/**
 * @return whether the handle has a nonzero generation.
 */
#define handle_is_nonnull(h) ((h).generation != 0)

/**
 * @return whether two handles have the same index and generation.
 */
#define handle_equal(a, b)                                                     \
  ((void)sizeof(0 ? (a) : (b)),                                                \
   (a).index == (b).index && (a).generation == (b).generation)

#endif // GINC_CORE_HANDLE_H
