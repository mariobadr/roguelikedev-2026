/**
 * @file alist.h
 */
#ifndef GINC_CONTAINER_ALIST_H
#define GINC_CONTAINER_ALIST_H

#include <SDL3/SDL_stdinc.h>

/**
 * Produces the struct tag name for a given tag identifier. Primarily for
 * internal use.
 */
#define alist_tag(tag) tag##_alist

/**
 * Refers to an alist type by its tag. The tag must match one previously
 * defined with alist_define or alist_define_as.
 */
#define alist(tag) struct alist_tag(tag)

/**
 * Defines an alist struct for a single-token element type, deriving the tag
 * automatically via alist_tag. For pointer or multi-word types use
 * alist_define_as instead.
 */
#define alist_define(type) alist_define_as(type, type)

/**
 * Defines an alist struct for element type T with an explicit tag. Required
 * when T is a pointer or multi-word type that alist_define cannot handle.
 */
#define alist_define_as(T, tag)                                                \
  struct alist_tag(tag)                                                        \
  {                                                                            \
    T* data;                                                                   \
    size_t len;                                                                \
    size_t cap;                                                                \
  }

/**
 * Allocates a zero-initialized alist with capacity for `n` elements.
 *
 * @param a pointer to an alist struct.
 * @param n Number of elements to allocate capacity for.
 *
 * @return whether allocation failed.
 */
#define alist_alloc(a, n)                                                      \
  ((a)->data = SDL_calloc((size_t)(n), sizeof(*(a)->data)),                    \
   (a)->cap = (a)->data ? (n) : 0,                                             \
   (a)->len = 0,                                                               \
   (a)->data != NULL)

/**
 * Frees the backing buffer and resets the alist to an empty state.
 */
#define alist_free(a)                                                          \
  (SDL_free((a)->data), (a)->data = NULL, (a)->len = 0, (a)->cap = 0)

/**
 * @return the number of elements currently in the alist.
 */
#define alist_len(a) ((a)->len)

/**
 * @return the maximum number of elements the alist can hold.
 */
#define alist_cap(a) ((a)->cap)

/**
 * @return a pointer to the element at i.
 */
#define alist_at(a, i) (&(a)->data[(i)])

/**
 * Double the capacity of the backing buffer.
 *
 * @return whether the buffer was successfully grown.
 */
bool
alist_grow(void** data, size_t* cap, size_t elem_size);

/**
 * @return pointer to the newly reserved element, or `NULL` if allocation
 * fails.
 */
#define alist_push(a)                                                          \
  ((a)->len < (a)->cap ||                                                      \
       alist_grow((void**)&(a)->data, &(a)->cap, sizeof(*(a)->data))           \
     ? &(a)->data[(a)->len++]                                                  \
     : NULL)

/**
 * Note: The pointer is only valid until the next alist_push overwrites the
 * slot.
 *
 * @return pointer to the removed element, or `NULL` if the alist is empty.
 */
#define alist_pop(a) ((a)->len > 0 ? &(a)->data[--(a)->len] : NULL)

/**
 * Reset the alist length to zero without modifying the backing buffer.
 */
#define alist_clear(a) ((a)->len = 0)

#endif // GINC_CONTAINER_ALIST_H
