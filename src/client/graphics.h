/**
 * @file graphics.h
 */
#ifndef GINC_ROGUELIKE_GRAPHICS_H
#define GINC_ROGUELIKE_GRAPHICS_H

#include "game/tile.h"

#include "client/render.h"

// forward declarations
struct rl_actor;
struct rl_item;

/**
 * @return how the tile should be rendered on the screen.
 */
struct rl_cell
rl_get_tile_gfx(enum rl_tile tile);

/**
 * @return how the item should be rendered on the screen.
 */
struct rl_cell
rl_get_item_gfx(struct rl_item const *item);

/**
 * @return how the actor should be rendered on the screen.
 */
struct rl_cell
rl_get_actor_gfx(struct rl_actor const* actor);

#endif // GINC_ROGUELIKE_GRAPHICS_H
