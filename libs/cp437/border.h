/**
 * @file border.h
 */
#ifndef GINC_CP437_BORDER_H
#define GINC_CP437_BORDER_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

#include "container/grid.h"

/**
 * The weight of a border stroke, in increasing precedence.
 */
enum cp437_border_weight
{
  CP437_BORDER_NONE,
  CP437_BORDER_SINGLE,
  CP437_BORDER_DOUBLE,
};

/**
 * The stroke weights of the four sides of a box.
 */
struct cp437_border_style
{
  enum cp437_border_weight top;
  enum cp437_border_weight right;
  enum cp437_border_weight bottom;
  enum cp437_border_weight left;
};

/**
 * The connections and retained stroke weights at one grid cell.
 */
struct cp437_border_cell
{
  /** The neighbouring cells connected to this cell. */
  Uint8 connections;
  /** The shared weight of east and west connections. */
  enum cp437_border_weight horizontal;
  /** The shared weight of north and south connections. */
  enum cp437_border_weight vertical;
};

/**
 * Positions outside the perimeter return an empty cell.
 * 
 * @return the border topology at a position on a box perimeter.
 */
struct cp437_border_cell
cp437_box_cell(SDL_Rect const* box,
               struct cp437_border_style const* style,
               int x,
               int y);

/**
 * A grid of border connections and weights with shared edges merged.
 */
grid_define_as(struct cp437_border_cell, cp437_border);

/**
 * Add a horizontal stroke, merging it with existing strokes.
 */
void
cp437_add_hline(grid(cp437_border) * borders,
                int x,
                int y,
                int length,
                enum cp437_border_weight weight);

/**
 * Add a vertical stroke, merging it with existing strokes.
 */
void
cp437_add_vline(grid(cp437_border) * borders,
                int x,
                int y,
                int length,
                enum cp437_border_weight weight);

/**
 * Add a box, merging connections and retaining the heavier weight on each
 * axis. A side with weight none adds no stroke.
 */
void
cp437_add_box(grid(cp437_border) * borders,
              SDL_Rect const* box,
              struct cp437_border_style const* style);

/**
 * Resolve a retained border cell to its CP437 glyph index.
 *
 * An empty cell resolves to CP437_SPACE.
 *
 * @return a CP437 glyph index.
 */
Uint8
cp437_border_glyph(struct cp437_border_cell const* cell);

#endif // GINC_CP437_BORDER_H
