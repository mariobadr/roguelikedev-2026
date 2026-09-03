/**
 * @file grid.h
 */
#ifndef GINC_CONTAINER_GRID_H
#define GINC_CONTAINER_GRID_H

#include <SDL3/SDL_stdinc.h>

/**
 * Produces the struct tag name for a given tag identifier. Primarily for
 * internal use.
 */
#define grid_tag(tag) tag##_grid

/**
 * Refers to a grid type by its tag. The tag must match one previously defined
 * with grid_define or grid_define_as.
 */
#define grid(tag) struct grid_tag(tag)

/**
 * Defines a grid struct for a single-token element type, deriving the tag
 * automatically via grid_tag. For pointer or multi-word types use
 * grid_define_as instead.
 */
#define grid_define(type) grid_define_as(type, type)

/**
 * Defines a grid struct for element type T with an explicit tag. Required when
 * T is a pointer or multi-word type that grid_define cannot handle.
 */
#define grid_define_as(T, tag)                                                 \
  struct grid_tag(tag)                                                         \
  {                                                                            \
    T* data;                                                                   \
    int width;                                                                 \
    int height;                                                                \
  }

/**
 * Allocates a zero-initialized grid with the given dimensions.
 *
 * @param g pointer to a grid struct.
 * @param w Width of the grid.
 * @param h Height of the grid.
 *
 * @return whether allocation succeeded.
 */
#define grid_alloc(g, w, h)                                                    \
  ((g)->data = SDL_calloc((size_t)(w) * (h), sizeof(*(g)->data)),              \
   (g)->width = (g)->data ? (w) : 0,                                           \
   (g)->height = (g)->data ? (h) : 0,                                          \
   (g)->data != NULL)

/**
 * Frees the backing buffer and resets the grid to an empty state.
 */
#define grid_free(g)                                                           \
  (SDL_free((g)->data), (g)->data = NULL, (g)->width = 0, (g)->height = 0)

/**
 * @return the width of the grid.
 */
#define grid_width(g) ((g)->width)

/**
 * @return the height of the grid.
 */
#define grid_height(g) ((g)->height)

/**
 * @return the number of elements in the grid.
 */
#define grid_count(g) ((size_t)(g)->width * (g)->height)

/**
 * @return whether the coordinates (x, y) are within the grid.
 */
#define grid_contains(g, x, y)                                                 \
  ((x) >= 0 && (y) >= 0 && (x) < (g)->width && (y) < (g)->height)

/**
 * @return the row-major array index of (x, y) in the grid.
 */
#define grid_index_of(g, x, y) ((size_t)(y) * (size_t)(g)->width + (size_t)(x))

/**
 * @return a pointer to the element at (x, y).
 */
#define grid_at(g, x, y) (&(g)->data[grid_index_of(g, x, y)])

/**
 * @return a pointer to the element at i in row-major order.
 */
#define grid_at_index(g, i) (&(g)->data[(i)])

/**
 * @return whether two grids have the same dimensions.
 */
#define grid_same_shape(a, b)                                                  \
  ((a)->width == (b)->width && (a)->height == (b)->height)

// define some common grid types
grid_define(int);
grid_define_as(bool, boolean);

#endif // GINC_CONTAINER_GRID_H
