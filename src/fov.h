/**
 * @file fov.h
 */
#ifndef GINC_ROGUELIKE_FOV_H
#define GINC_ROGUELIKE_FOV_H

#include <SDL3/SDL_rect.h>

// forward declarations
struct rl_map;

/**
 * Update out with a field of view from origin.
 *
 * @param map       a map to inspect for calculating the field of view
 * @param origin    the centre point
 * @param radius    the radius of the circle around origin
 * @param out       The visibility of the tiles in map
 */
void
rl_compute_fov(struct rl_map const* map,
               SDL_Point origin,
               int radius,
               bool* out);

#endif // GINC_ROGUELIKE_FOV_H