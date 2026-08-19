/**
 * @file fov.h
 */
#ifndef GINC_ROGUELIKE_FOV_H
#define GINC_ROGUELIKE_FOV_H

#include <SDL3/SDL_rect.h>

// forward declarations
struct rl_map;

/**
 * Calculate the field of view from origin and save it to out.
 *
 * Existing values in out are cleared before visibility is calculated. The
 * output array must have space for at least map->width * map->height elements.
 * Origin must be within map and radius must not be negative.
 *
 * @param map    The map to inspect.
 * @param origin The centre of the field of view, in tile coordinates.
 * @param radius The maximum view distance, in tiles.
 * @param out    The output array of visible tiles.
 */
void
rl_compute_fov(struct rl_map const* map,
               SDL_Point origin,
               int radius,
               bool* out);

#endif // GINC_ROGUELIKE_FOV_H
