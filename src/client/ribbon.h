/**
 * @file ribbon.h
 */
#ifndef GINC_ROGUELIKE_RIBBON_H
#define GINC_ROGUELIKE_RIBBON_H

#include <SDL3/SDL_rect.h>

#include "client/text.h"

// external forward declarations
typedef struct SDL_Renderer SDL_Renderer;

// forward declarations
struct gfx_tileset;

/**
 * The regions of the ribbon that hold text.
 */
enum rl_ribbon_slot
{
  RL_RIBBON_LEFT,
  RL_RIBBON_CENTRE,
  RL_RIBBON_RIGHT,
  RL_RIBBON_SLOT_COUNT, //< number of slots
};

/**
 * The text of every ribbon slot, indexed by rl_ribbon_slot.
 *
 * An empty rl_text leaves its slot blank.
 */
struct rl_ribbon_content
{
  /** The text of each slot. */
  struct rl_text text[RL_RIBBON_SLOT_COUNT];
};

/**
 * A line of status text with left, centre and right slots.
 */
struct rl_ribbon
{
  /** The screen region the ribbon is drawn in. */
  SDL_FRect viewport;
  /** The width of one glyph, in pixels. */
  float glyph_width;
  /** The height of one line of text, in pixels. */
  float line_height;

  /** The screen region each slot's text is clipped to. */
  SDL_Rect clips[RL_RIBBON_SLOT_COUNT];
  /** The screen position where each slot's text starts. */
  SDL_FPoint origins[RL_RIBBON_SLOT_COUNT];

  /** The text being shown. */
  struct rl_ribbon_content content;
};

/**
 * Initialise ribbon to draw within viewport, sized for font.
 */
void
rl_init_ribbon(struct rl_ribbon* ribbon,
               SDL_FRect const* viewport,
               struct gfx_tileset const* font);

/**
 * Set the text shown in the ribbon.
 */
void
rl_set_ribbon_content(struct rl_ribbon* ribbon,
                      struct rl_ribbon_content const* content);

/**
 * Draw the ribbon.
 */
void
rl_draw_ribbon(struct rl_ribbon const* ribbon,
               SDL_Renderer* renderer,
               struct gfx_tileset const* font);

#endif // GINC_ROGUELIKE_RIBBON_H
