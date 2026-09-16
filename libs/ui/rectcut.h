/**
 * @file rectcut.h
 *
 * Rect-cut helpers for slicing strips from the edges of a rectangle.
 */
#ifndef GINC_UI_RECTCUT_H
#define GINC_UI_RECTCUT_H

#include <SDL3/SDL_rect.h>

/**
 * Remove a strip of height h from the top of bounds and return it.
 *
 * @param bounds The rectangle to cut; shrunk in place by h on the top.
 * @param h      Height of the strip in pixels.
 *
 * @return The removed top strip.
 */
static inline SDL_FRect
ui_cut_top(SDL_FRect* bounds, float h)
{
  SDL_FRect s = { bounds->x, bounds->y, bounds->w, h };
  bounds->y += h;
  bounds->h -= h;
  return s;
}

/**
 * Remove a strip of height h from the bottom of bounds and return it.
 *
 * @param bounds The rectangle to cut; shrunk in place by h on the bottom.
 * @param h      Height of the strip in pixels.
 *
 * @return The removed bottom strip.
 */
static inline SDL_FRect
ui_cut_bottom(SDL_FRect* bounds, float h)
{
  SDL_FRect s = { bounds->x, bounds->y + bounds->h - h, bounds->w, h };
  bounds->h -= h;
  return s;
}

/**
 * Remove a strip of width w from the left of bounds and return it.
 *
 * @param bounds The rectangle to cut; shrunk in place by w on the left.
 * @param w      Width of the strip in pixels.
 *
 * @return The removed left strip.
 */
static inline SDL_FRect
ui_cut_left(SDL_FRect* bounds, float w)
{
  SDL_FRect s = { bounds->x, bounds->y, w, bounds->h };
  bounds->x += w;
  bounds->w -= w;
  return s;
}

/**
 * Remove a strip of width w from the right of bounds and return it.
 *
 * @param bounds The rectangle to cut; shrunk in place by w on the right.
 * @param w      Width of the strip in pixels.
 *
 * @return The removed right strip.
 */
static inline SDL_FRect
ui_cut_right(SDL_FRect* bounds, float w)
{
  SDL_FRect s = { bounds->x + bounds->w - w, bounds->y, w, bounds->h };
  bounds->w -= w;
  return s;
}

#endif // GINC_UI_RECTCUT_H
