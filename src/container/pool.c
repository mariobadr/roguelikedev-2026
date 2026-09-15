#include "pool.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

static void*
slot_at(void* slots, size_t slot_size, Uint32 index)
{
  return (char*)slots + (size_t)index * slot_size;
}

static struct pool_slot_state*
state_of(void* slot)
{
  return (struct pool_slot_state*)slot;
}

bool
pool_reserve_impl(void** slots,
                  size_t slot_size,
                  Uint32* cap,
                  Uint32* free_head,
                  size_t n)
{
  if (n <= *cap) {
    return true;
  }

  void* const new_slots = SDL_realloc(*slots, n * slot_size);
  if (new_slots == NULL) {
    SDL_Log("SDL_realloc failed: %s", SDL_GetError());
    return false;
  }

  Uint32 const old_cap = *cap;
  Uint32 const new_cap = (Uint32)n;

  void* const first_new_slot = slot_at(new_slots, slot_size, old_cap);
  size_t const new_slots_size = (size_t)(new_cap - old_cap) * slot_size;
  SDL_memset(first_new_slot, 0, new_slots_size);

  for (Uint32 i = old_cap; i < new_cap; ++i) {
    void* const slot = slot_at(new_slots, slot_size, i);
    struct pool_slot_state* const state = state_of(slot);

    state->generation = 1;
    state->next_free = i + 1 < new_cap ? i + 1 : *free_head;
    state->occupied = false;
  }

  *slots = new_slots;
  *cap = new_cap;
  *free_head = old_cap;
  return true;
}

bool
pool_acquire_impl(void* slots,
                  size_t slot_size,
                  Uint32* len,
                  Uint32* free_head,
                  Uint32* out_index,
                  Uint32* out_generation)
{
  if (*free_head == POOL_NIL) {
    *out_index = 0;
    *out_generation = 0;
    return false;
  }

  Uint32 const index = *free_head;
  void* const slot = slot_at(slots, slot_size, index);
  struct pool_slot_state* const state = state_of(slot);
  SDL_assert(!state->occupied);

  *free_head = state->next_free;
  state->occupied = true;
  ++*len;

  *out_index = index;
  *out_generation = state->generation;
  return true;
}

bool
pool_release_impl(void* slots,
                  size_t slot_size,
                  Uint32* len,
                  Uint32 cap,
                  Uint32* free_head,
                  Uint32 index,
                  Uint32 generation)
{
  if (index >= cap) {
    return false;
  }

  void* const slot = slot_at(slots, slot_size, index);
  struct pool_slot_state* const state = state_of(slot);

  if (!state->occupied || state->generation != generation) {
    return false;
  }

  state->occupied = false;
  --*len;

  // An exhausted generation is never reused, so stale handles cannot match a
  // later occupant.
  if (state->generation == SDL_MAX_UINT32) {
    return true;
  }

  ++state->generation;
  state->next_free = *free_head;
  *free_head = index;

  return true;
}
