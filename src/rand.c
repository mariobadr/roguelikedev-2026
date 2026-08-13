/*  Written in 2018 by David Blackman and Sebastiano Vigna (vigna@acm.org)

To the extent possible under law, the author has dedicated all copyright
and related and neighboring rights to this software to the public domain
worldwide.

Permission to use, copy, modify, and/or distribute this software for any
purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR
IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

#include "rand.h"

#include <SDL3/SDL_assert.h>

static inline Uint64
rotl(Uint64 const x, int k)
{
  return (x << k) | (x >> (64 - k));
}

void
rand_seed(struct rand_state* state, Uint64 seed)
{
  for (int i = 0; i < 4; i++) {
    seed += 0x9e3779b97f4a7c15ULL;
    Uint64 z = seed;
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
    z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
    state->s[i] = z ^ (z >> 31);
  }
}

Uint64
rand_next(struct rand_state* state)
{
  Uint64 const result = rotl(state->s[1] * 5, 7) * 9;

  Uint64 const t = state->s[1] << 17;

  state->s[2] ^= state->s[0];
  state->s[3] ^= state->s[1];
  state->s[1] ^= state->s[2];
  state->s[0] ^= state->s[3];

  state->s[2] ^= t;

  state->s[3] = rotl(state->s[3], 45);

  return result;
}

Uint64
rand_next_up_to(struct rand_state* state, Uint64 n)
{
  if (n == 0) {
    return 0;
  }

  Uint64 x, r;
  do {
    x = rand_next(state);
    r = x % n;
  } while (x - r > (UINT64_MAX - n + 1));

  return r;
}

Sint64
rand_next_between(struct rand_state* state, Sint64 lo, Sint64 hi)
{
  SDL_assert(hi > lo);

  Uint64 span = (hi - lo) + 1;
  return lo + (Sint64)rand_next_up_to(state, span);
}

/* This is the jump function for the generator. It is equivalent
   to 2^128 calls to next(); it can be used to generate 2^128
   non-overlapping subsequences for parallel computations. */

void
rand_jump(struct rand_state* state)
{
  static Uint64 const JUMP[] = { 0x180ec6d33cfd0aba,
                                 0xd5a61266f0c9392c,
                                 0xa9582618e03fc9aa,
                                 0x39abdc4529b1661c };

  Uint64 s0 = 0;
  Uint64 s1 = 0;
  Uint64 s2 = 0;
  Uint64 s3 = 0;
  for (int i = 0; i < sizeof JUMP / sizeof *JUMP; i++)
    for (int b = 0; b < 64; b++) {
      if (JUMP[i] & UINT64_C(1) << b) {
        s0 ^= state->s[0];
        s1 ^= state->s[1];
        s2 ^= state->s[2];
        s3 ^= state->s[3];
      }
      rand_next(state);
    }

  state->s[0] = s0;
  state->s[1] = s1;
  state->s[2] = s2;
  state->s[3] = s3;
}
