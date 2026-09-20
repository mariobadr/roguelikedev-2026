/**
 * @file layout.h
 */
#ifndef GINC_ROGUELIKE_LAYOUT_H
#define GINC_ROGUELIKE_LAYOUT_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "container/array.h"

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
  /** Index of the first room this corridor connects. */
  int room_a;
  /** Index of the second room this corridor connects. */
  int room_b;
};

/**
 * An array of rectangles.
 */
array_define_as(SDL_Rect, rl_room);

/**
 * An array of corridors.
 */
array_define_as(struct rl_corridor, rl_corridor);

/**
 * Where rooms and corridors can be found.
 */
struct rl_layout
{
  /** The rooms in the layout. */
  array(rl_room) rooms;
  /** The corridors in the layout. */
  array(rl_corridor) corridors;
};

/**
 * Generate a random layout of rooms and corridors within width by height
 * cells.
 *
 * @return whether allocation succeeded.
 */
bool
rl_init_layout(struct rl_layout* layout,
               int width,
               int height,
               struct rand_state* rng);

/**
 * Free the layout.
 */
void
rl_free_layout(struct rl_layout* layout);

#endif // GINC_ROGUELIKE_LAYOUT_H
