/**
 * @file pool.h
 */
#ifndef GINC_CONTAINER_POOL_H
#define GINC_CONTAINER_POOL_H

#include <stddef.h>

#include <SDL3/SDL_stdinc.h>

#include "core/handle.h"

/**
 * Marks the end of a pool's free list. A pool can hold at most this many slots.
 */
#define POOL_NIL SDL_MAX_UINT32

/**
 * Produces the struct tag name of a pool for a given tag identifier. Primarily
 * for internal use.
 */
#define pool_tag(tag) tag##_pool

/**
 * Produces the struct tag name of a pool slot for a given tag identifier.
 * Primarily for internal use.
 */
#define pool_slot_tag(tag) tag##_pool_slot

/**
 * Refers to a pool type by its tag. The tag must match one previously defined
 * with pool_define or pool_define_as.
 */
#define pool(tag) struct pool_tag(tag)

/**
 * Defines a pool struct and handle type for a single-token element type,
 * deriving the tag automatically. For pointer or multi-word types use
 * pool_define_as instead.
 */
#define pool_define(type) pool_define_as(type, type)

/**
 * Metadata state for a slot in a pool.
 */
struct pool_slot_state
{
  Uint32 generation;
  Uint32 next_free;
  bool occupied;
};

/**
 * Note: this expects that the tag has been defined as a handle using
 * handle_define
 */
#define pool_define_as(T, tag)                                                 \
  struct pool_slot_tag(tag)                                                    \
  {                                                                            \
    struct pool_slot_state state;                                              \
    T value;                                                                   \
  };                                                                           \
  struct pool_tag(tag)                                                         \
  {                                                                            \
    struct pool_slot_tag(tag) * slots;                                         \
    Uint32 len;                                                                \
    Uint32 cap;                                                                \
    Uint32 free_head;                                                          \
  }

/**
 * Grows the slot buffer to hold at least n slots. Primarily for internal use;
 * see pool_reserve.
 */
bool
pool_reserve_impl(void** slots,
                  size_t slot_size,
                  Uint32* cap,
                  Uint32* free_head,
                  size_t n);

/**
 * Occupies the next free slot. Primarily for internal use; see pool_acquire.
 */
bool
pool_acquire_impl(void* slots,
                  size_t slot_size,
                  Uint32* len,
                  Uint32* free_head,
                  Uint32* out_index,
                  Uint32* out_generation);

/**
 * Frees the slot referred to by index and generation. Primarily for internal
 * use; see pool_release.
 */
bool
pool_release_impl(void* slots,
                  size_t slot_size,
                  Uint32* len,
                  Uint32 cap,
                  Uint32* free_head,
                  Uint32 index,
                  Uint32 generation);

/**
 * Grow the pool to hold at least n elements. Does nothing if the pool can
 * already hold `n` elements.
 *
 * @return whether the pool can hold n elements. On failure the pool is
 * unchanged.
 */
#define pool_reserve(p, n)                                                     \
  pool_reserve_impl((void**)&(p)->slots,                                       \
                    sizeof(*(p)->slots),                                       \
                    &(p)->cap,                                                 \
                    &(p)->free_head,                                           \
                    (size_t)(n))

/**
 * Allocate an empty pool with capacity for n elements.
 *
 * @param p pointer to a pool struct.
 * @param n Number of elements to allocate capacity for.
 *
 * @return whether allocation succeeded.
 */
#define pool_alloc(p, n)                                                       \
  ((p)->slots = NULL,                                                          \
   (p)->len = 0,                                                               \
   (p)->cap = 0,                                                               \
   (p)->free_head = POOL_NIL,                                                  \
   pool_reserve(p, n))

/**
 * Free the backing buffer and reset the pool to an empty state.
 */
#define pool_free(p)                                                           \
  (SDL_free((p)->slots),                                                       \
   (p)->slots = NULL,                                                          \
   (p)->len = 0,                                                               \
   (p)->cap = 0,                                                               \
   (p)->free_head = POOL_NIL)

/**
 * @return the number of "live" elements in the pool.
 */
#define pool_len(p) ((p)->len)

/**
 * @return the number of slots in the pool.
 */
#define pool_cap(p) ((p)->cap)

/**
 * @return whether no slot is available to acquire.
 */
#define pool_full(p) ((p)->free_head == POOL_NIL)

/**
 * Occupies a free slot and writes its handle to out.
 *
 * @param p pointer to a pool struct.
 * @param out pointer to a handle; invalid if the pool is full.
 *
 * @return pointer to the new element, or `NULL` if the pool is full.
 */
#define pool_acquire(p, out)                                                   \
  (pool_acquire_impl((p)->slots,                                               \
                     sizeof(*(p)->slots),                                      \
                     &(p)->len,                                                \
                     &(p)->free_head,                                          \
                     &(out)->index,                                            \
                     &(out)->generation)                                       \
     ? &(p)->slots[(out)->index].value                                         \
     : NULL)

/**
 * Frees the element referred to by h, invalidating the handle.
 *
 * @return whether a live element was released.
 */
#define pool_release(p, h)                                                     \
  pool_release_impl((p)->slots,                                                \
                    sizeof(*(p)->slots),                                       \
                    &(p)->len,                                                 \
                    (p)->cap,                                                  \
                    &(p)->free_head,                                           \
                    (h).index,                                                 \
                    (h).generation)

/**
 * Warning: The pointer is only valid until the element is released or the pool
 * grows.
 *
 * @return pointer to the element referred to by h, or `NULL` if h is
 * invalid/stale.
 */
#define pool_get(p, h)                                                         \
  ((h).index < (p)->cap && (p)->slots[(h).index].state.occupied &&             \
       (p)->slots[(h).index].state.generation == (h).generation                \
     ? &(p)->slots[(h).index].value                                            \
     : NULL)

/**
 * @return pointer to the live element in slot i.
 */
#define pool_at_index(p, i)                                                    \
  ((p)->slots[(i)].state.occupied ? &(p)->slots[(i)].value : NULL)

/**
 * Writes the handle for slot i to out.
 */
#define pool_handle_at(p, i, out)                                              \
  ((void)((out)->index = (Uint32)(i),                                          \
          (out)->generation = (p)->slots[(i)].state.generation))

#endif // GINC_CONTAINER_POOL_H
