/**
 * @file anchor.h
 *
 * Anchor flags and position resolution for placing elements within a rectangle.
 */
#ifndef GINC_UI_ANCHOR_H
#define GINC_UI_ANCHOR_H

#include <SDL3/SDL_rect.h>
#include <SDL3/SDL_stdinc.h>

/**
 * Bit flags controlling which edge or axis of the bounds rectangle an element
 * is anchored to.
 *
 * Combine one horizontal flag with one vertical flag to form a complete anchor,
 * or use the pre-built UI_ANCHOR_* macros.
 */
enum ui_anchor_flag
{
  UI_ANCHOR_LEFT = 0x01,     /**< Align left edge to bounds left. */
  UI_ANCHOR_RIGHT = 0x02,    /**< Align right edge to bounds right. */
  UI_ANCHOR_CENTRE_X = 0x04, /**< Centre horizontally within bounds. */

  UI_ANCHOR_TOP = 0x10,      /**< Align top edge to bounds top. */
  UI_ANCHOR_BOTTOM = 0x20,   /**< Align bottom edge to bounds bottom. */
  UI_ANCHOR_CENTRE_Y = 0x40, /**< Centre vertically within bounds. */
};

/** Anchor to the top-left corner. */
#define UI_ANCHOR_TOP_LEFT (UI_ANCHOR_TOP | UI_ANCHOR_LEFT)
/** Anchor to the top edge, centred horizontally. */
#define UI_ANCHOR_TOP_CENTRE (UI_ANCHOR_TOP | UI_ANCHOR_CENTRE_X)
/** Anchor to the top-right corner. */
#define UI_ANCHOR_TOP_RIGHT (UI_ANCHOR_TOP | UI_ANCHOR_RIGHT)
/** Anchor to the left edge, centred vertically. */
#define UI_ANCHOR_MID_LEFT (UI_ANCHOR_CENTRE_Y | UI_ANCHOR_LEFT)
/** Anchor to the centre of the bounds. */
#define UI_ANCHOR_CENTRE (UI_ANCHOR_CENTRE_Y | UI_ANCHOR_CENTRE_X)
/** Anchor to the right edge, centred vertically. */
#define UI_ANCHOR_MID_RIGHT (UI_ANCHOR_CENTRE_Y | UI_ANCHOR_RIGHT)
/** Anchor to the bottom-left corner. */
#define UI_ANCHOR_BOT_LEFT (UI_ANCHOR_BOTTOM | UI_ANCHOR_LEFT)
/** Anchor to the bottom edge, centred horizontally. */
#define UI_ANCHOR_BOT_CENTRE (UI_ANCHOR_BOTTOM | UI_ANCHOR_CENTRE_X)
/** Anchor to the bottom-right corner. */
#define UI_ANCHOR_BOT_RIGHT (UI_ANCHOR_BOTTOM | UI_ANCHOR_RIGHT)

/**
 * Screen-space position expressed as an anchor point within a bounds rectangle
 * plus a pixel offset from that anchor.
 */
struct ui_position
{
  Uint8 anchor;      /**< Combination of ui_anchor_flag bits. */
  SDL_FPoint offset; /**< Pixel offset applied after anchoring. */
};

/**
 * Compute the destination rectangle for an element within a bounds rectangle.
 *
 * @param p      The anchor and offset describing where within bounds the
 *               element should be placed.
 * @param bounds The container rectangle to anchor within.
 * @param w      Width of the element in pixels.
 * @param h      Height of the element in pixels.
 *
 * @return The resolved destination rectangle.
 */
SDL_FRect
ui_resolve(struct ui_position p, SDL_FRect const *bounds, float w, float h);

#endif // GINC_UI_ANCHOR_H
