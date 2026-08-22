/**
 * @file layout.h
 */
#ifndef GINC_ROGUELIKE_LAYOUT_H
#define GINC_ROGUELIKE_LAYOUT_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

// forward declarations
struct rand_state;

/**
 * A straight or L-shaped corridor.
 */
struct rl_corridor
{
  /** The maximum possible segments. */
  SDL_Rect segments[2];
  /** The actual number of segments. */
  int segment_count;
};

struct rl_layout
{
  /** The rooms in the layout. */
  SDL_Rect* rooms;
  /** The number of rooms in the layout. */
  int room_count;
  /** The corridors in the layout. */
  struct rl_corridor* corridors;
  /** The number of corridors. */
  int corridor_count;
};

bool
rl_init_layout(struct rl_layout* layout,
               int width,
               int height,
               struct rand_state* rng);

void
rl_free_layout(struct rl_layout* layout);

#endif // GINC_ROGUELIKE_LAYOUT_H
