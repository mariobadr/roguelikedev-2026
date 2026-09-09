/**
 * @file layout.h
 *
 * Column and row layout for sequences of uniformly sized items.
 */
#ifndef GINC_UI_LAYOUT_H
#define GINC_UI_LAYOUT_H

#include <SDL3/SDL_rect.h>

#include "ui/anchor.h"

/**
 * Describes a sequence of uniformly sized items to be laid out with equal
 * spacing between them.
 */
struct ui_strip
{
  int count;         /**< Number of items. */
  float item_width;  /**< Width of each item in pixels. */
  float item_height; /**< Height of each item in pixels. */
  float gap;         /**< Space between adjacent items in pixels. */
};

/**
 * Compute destination rectangles for a vertical column of uniformly sized
 * items anchored within a bounds rectangle.
 *
 * @param p      The anchor and offset describing where the column is placed
 *               within bounds.
 * @param bounds The container rectangle to anchor within.
 * @param strip  The item dimensions, count, and gap.
 * @param rects  Output array of at least strip.count SDL_FRect values.
 */
void
ui_layout_column(struct ui_position p,
                 SDL_FRect const* bounds,
                 struct ui_strip strip,
                 SDL_FRect* rects);

/**
 * Compute destination rectangles for a horizontal row of uniformly sized
 * items anchored within a bounds rectangle.
 *
 * @param p      The anchor and offset describing where the row is placed
 *               within bounds.
 * @param bounds The container rectangle to anchor within.
 * @param strip  The item dimensions, count, and gap.
 * @param rects  Output array of at least strip.count SDL_FRect values.
 */
void
ui_layout_row(struct ui_position p,
              SDL_FRect const* bounds,
              struct ui_strip strip,
              SDL_FRect* rects);

#endif // GINC_UI_LAYOUT_H
