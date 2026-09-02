/**
 * @file graphics.h
 */
#ifndef GINC_ROGUELIKE_GRAPHICS_H
#define GINC_ROGUELIKE_GRAPHICS_H

#include "render.h"
#include "game/tile.h"

// forward declarations
struct rl_entity;

struct rl_gfx_tile
rl_get_tile_gfx(enum rl_tile tile);

struct rl_gfx_tile
rl_get_entity_gfx(struct rl_entity const* entity);

#endif // GINC_ROGUELIKE_GRAPHICS_H
