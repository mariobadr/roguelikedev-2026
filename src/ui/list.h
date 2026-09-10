/**
 * @file list.h
 */
#ifndef GINC_UI_LIST_H
#define GINC_UI_LIST_H

#include <SDL3/SDL_rect.h>

#include "ui/layout.h"

/**
 * A scrollable list view.
 */
struct ui_list
{
  /** The bounds where the list is displayed. */
  SDL_FRect bounds;
  /** An array of rectangles where visible list items go. */
  SDL_FRect* slots;
  /** Maximum number of rectangles available in slots. */
  int max_slots;
  /** Number of list items that can be displayed. */
  int slot_count;

  /** Height of each item, in pixels. */
  float item_height;
  /** Gap between items, in pixels. */
  float gap;

  /** Preferred index of the first visible list item. */
  int offset;
};

/**
 * Initialize a list.
 */
void
ui_list_init(struct ui_list* list,
             SDL_FRect const* bounds,
             SDL_FRect* slots,
             int max_slots,
             float item_height,
             float gap);

/**
 * Resize the bounds of a list.
 */
void
ui_list_resize(struct ui_list* list, SDL_FRect const* bounds);

/**
 * @return the greatest valid list offset for a model of count items.
 */
int
ui_list_max_offset(struct ui_list const* list, int count);

/**
 * @return the current effective offset for a model of count items.
 */
int
ui_list_offset(struct ui_list const* list, int count);

/**
 * Scroll to an item offset, clamped to the valid range.
 */
void
ui_list_scroll_to(struct ui_list* list, int count, int offset);

/**
 * Scroll by an amount relative to the current effective offset.
 */
void
ui_list_scroll_by(struct ui_list* list, int count, int amount);

/**
 * Scroll as necessary to make index visible.
 */
void
ui_list_ensure_visible(struct ui_list* list, int count, int index);

/**
 * @return the exclusive end of the visible model range.
 */
int
ui_list_end(struct ui_list const* list, int count);

#endif // GINC_UI_LIST_H