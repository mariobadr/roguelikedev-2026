/**
 * @file result.h
 */
#ifndef GINC_ROGUELIKE_SERIALIZE_RESULT_H
#define GINC_ROGUELIKE_SERIALIZE_RESULT_H

#include <SDL3/SDL_iostream.h>

/**
 * The outcome of a read.
 */
enum rl_read_result
{
  RL_READ_OK,      //< read succeeded
  RL_READ_CORRUPT, //< the input fails validation
  RL_READ_ERROR,   //< allocation or underlying I/O failed
};

/**
 * @return the result for a read from src that just failed.
 */
static inline enum rl_read_result
rl_read_failure(SDL_IOStream* src)
{
  return SDL_GetIOStatus(src) == SDL_IO_STATUS_EOF ? RL_READ_CORRUPT
                                                   : RL_READ_ERROR;
}

/**
 * Return from the enclosing function with the appropriate rl_read_result if
 * call, a read from src, did not succeed.
 */
#define RL_READ_OR_FAIL(src, call)                                             \
  do {                                                                         \
    if (!(call)) {                                                             \
      return rl_read_failure(src);                                             \
    }                                                                          \
  } while (0)

#endif // GINC_ROGUELIKE_SERIALIZE_RESULT_H
