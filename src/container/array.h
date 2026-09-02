/**
 * @file array.h
 */
#ifndef GINC_CONTAINER_ARRAY_H
#define GINC_CONTAINER_ARRAY_H

#include <SDL3/SDL_stdinc.h>

/**
 * Produces the struct tag name for a given tag identifier. Primarily for
 * internal use.
 */
#define array_tag(tag) tag##_array

/**
 * Refers to an array type by its tag. The tag must match one previously
 * defined with array_define or array_define_as.
 */
#define array(tag) struct array_tag(tag)

/**
 * Defines an array struct for a single-token element type, deriving the tag
 * automatically via array_tag. For pointer or multi-word types use
 * array_define_as instead.
 */
#define array_define(type) array_define_as(type, type)

/**
 * Defines an array struct for element type T with an explicit tag. Required
 * when T is a pointer or multi-word type that array_define cannot handle.
 */
#define array_define_as(T, tag)                                                \
  struct array_tag(tag)                                                        \
  {                                                                            \
    T* data;                                                                   \
    size_t len;                                                                \
    size_t cap;                                                                \
  }

/**
 * Initializes an array to use an existing buffer.
 *
 * @param a pointer to an array struct.
 * @param buf pointer to the backing buffer.
 * @param n Number of elements the buffer can hold.
 */
#define array_init(a, buf, n) ((a)->data = (buf), (a)->len = 0, (a)->cap = (n))

/**
 * Allocates a zero-initialized array with capacity for `n` elements.
 *
 * @param a pointer to an alist struct.
 * @param n Number of elements to allocate capacity for.
 *
 * @return whether allocation failed.
 */
#define array_alloc(a, n)                                                      \
  ((a)->data = SDL_calloc((size_t)(n), sizeof(*(a)->data)),                    \
   (a)->cap = (a)->data ? (n) : 0,                                             \
   (a)->len = 0,                                                               \
   (a)->data != NULL)

/**
 * Frees the backing buffer and resets the array to an empty state.
 */
#define array_free(a)                                                          \
  (SDL_free((a)->data), (a)->data = NULL, (a)->len = 0, (a)->cap = 0)

/**
 * @return the number of elements currently in the array.
 */
#define array_len(a) ((a)->len)

/**
 * @return the maximum number of elements the array can hold.
 */
#define array_cap(a) ((a)->cap)

/**
 * @return whether the array has any elements.
 */
#define array_empty(a) ((a)->len == 0)

/**
 * @return whether the array has reached its capacity.
 */
#define array_full(a) ((a)->len >= (a)->cap)

/**
 * @return a pointer to the element at i.
 */
#define array_at(a, i) (&(a)->data[(i)])

/**
 * @return pointer to the newly reserved element, or `NULL` if the array is
 * full.
 */
#define array_push(a) ((a)->len < (a)->cap ? &(a)->data[(a)->len++] : NULL)

/**
 * Note: The pointer is only valid until the next array_push overwrites the
 * slot.
 *
 * @return pointer to the removed element, or `NULL` if the array is empty.
 */
#define array_pop(a) ((a)->len > 0 ? &(a)->data[--(a)->len] : NULL)

/**
 * Reset the array length to zero without modifying the backing buffer.
 */
#define array_clear(a) ((a)->len = 0)

// define some common array types
array_define_as(bool, boolean);

#endif // GINC_CONTAINER_ARRAY_H
