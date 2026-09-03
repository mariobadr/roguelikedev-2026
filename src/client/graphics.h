/**
 * @file graphics.h
 */
#ifndef GINC_ROGUELIKE_GRAPHICS_H
#define GINC_ROGUELIKE_GRAPHICS_H

#include "game/tile.h"

#include "client/render.h"
#include "client/text.h"

// forward declarations
struct rl_actor;

/**
 * A graphical tile.
 */
struct rl_gfx_tile
{
  /** Glyph index into the font. */
  Uint8 glyph;
  /** Foreground tint applied to the glyph. */
  SDL_FColor fg;
  /** Background colour of the tile. */
  SDL_FColor bg;
};

struct rl_gfx_tile
rl_get_tile_gfx(enum rl_tile tile);

struct rl_gfx_tile
rl_get_actor_gfx(struct rl_actor const* actor);

struct rl_gfx_tile
rl_get_text_gfx(enum rl_text_style style);

#endif // GINC_ROGUELIKE_GRAPHICS_H
