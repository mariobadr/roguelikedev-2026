#include "alist.h"

#include <SDL3/SDL_assert.h>
#include <SDL3/SDL_error.h>
#include <SDL3/SDL_log.h>

bool
alist_grow(void** data, size_t* cap, size_t elem_size)
{
  SDL_assert(*cap > 0);

  size_t const new_cap = *cap * 2;
  void* const new_data = SDL_realloc(*data, new_cap * elem_size);
  if (new_data == NULL) {
    SDL_Log("SDL_realloc failed: %s", SDL_GetError());
    return false;
  }

  *data = new_data;
  *cap = new_cap;
  return true;
}
